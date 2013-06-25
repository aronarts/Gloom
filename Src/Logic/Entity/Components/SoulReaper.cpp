/**
@file SoulReaper.cpp

Contiene la implementaci�n del componente que representa al soulReaper.
 
@see Logic::CSoulReaper
@see Logic::CShootRaycast

@author Jose Antonio Garc�a Y��ez
@date Febrero,2013
*/

#include "SoulReaper.h"
#include "Physics/RaycastHit.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Server.h"
#include "Map/MapEntity.h"
#include "Physics/Server.h"
#include "Graphics/Server.h"
#include "Graphics/Scene.h"
#include "Logic/Entity/Components/ArrayGraphics.h"
#include "Logic/Entity/Components/Life.h"
#include "Logic/Entity/Components/PullingMovement.h"
#include "Logic/Entity/Components/SpawnItemManager.h"
#include "Logic/Entity/Components/Graphics.h"
#include "Logic/Entity/Components/PhysicDynamicEntity.h"
#include "Logic/Entity/Components/PhysicStaticEntity.h"

#include "Logic/Messages/MessageControl.h"
#include "Logic/Messages/MessageActivate.h"
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageAddForcePlayer.h"
#include "Logic/GameNetMsgManager.h"

#include "Graphics/Camera.h"

#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreManualObject.h>

namespace Logic {
	IMP_FACTORY(CSoulReaper);

	CSoulReaper::~CSoulReaper() {
		// Nada que hacer
	}

	//__________________________________________________________________
	
	bool CSoulReaper::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		if(!IWeapon::spawn(entity,map,entityInfo)) return false;

		
		_shotsDistanceSecondaryFire = entityInfo->getFloatAttribute(_weaponName + "ShotsDistanceSecondaryFire");

		_shotsDistance = entityInfo->getFloatAttribute(_weaponName + "ShotsDistance");

		_defaultPrimaryFireDamage = _primaryFireDamage = entityInfo->getFloatAttribute(_weaponName + "PrimaryFireDamage");

		_defaultPrimaryFireCooldown = _primaryFireCooldown = entityInfo->getFloatAttribute(_weaponName+"PrimaryFireCooldown") * 1000;


		return true;
	}

	//__________________________________________________________________

	void CSoulReaper::onActivate() {
		
		//Reiniciamos el cooldown
		reduceCooldown(0);
		//y los da�os
		amplifyDamage(0);
	}

	//__________________________________________________________________

	void CSoulReaper::resetAmmo() {
		//si yo soy el weapon
		_currentAmmo = 1;
	} // resetAmmo
	//__________________________________________________________________



	void CSoulReaper::primaryFire() {
		

		_primaryFireTimer = _primaryFireCooldown;
	
		Vector3 direction = _entity->getOrientation()*Vector3::NEGATIVE_UNIT_Z;
		direction.normalise();

		Vector3 origin = _entity->getPosition()+Vector3(0.0f,_heightShoot,0.0f);

		Ray ray(origin, direction);
			
		std::vector <Physics::CRaycastHit> hits;
		Physics::CServer::getSingletonPtr()->raycastMultiple(ray, _shotsDistance, hits,true, Physics::CollisionGroup::ePLAYER  | Physics::CollisionGroup::eWORLD);
		for (auto it = hits.begin(); it < hits.end(); ++it){
			//Si tocamos el mundo no continuamos viendo hits
			if((*it).entity->getType().compare("World")==0){
				return;
			}
			if((*it).entity->getEntityID() != _entity->getEntityID()){
				std::shared_ptr<CMessageDamaged> m = std::make_shared<CMessageDamaged>();
				m->setDamage(_primaryFireDamage);
				m->setEnemy(_entity);
				(*it).entity->emitMessage(m);
				// esto es para que salga una vez que ya le ha dao a alguien que no eres tu mismo.
				return;
			}
		}
	
	} // primaryFire
	//__________________________________________________________________

	void CSoulReaper::secondaryFire() {
		

		//primero preguntamos si podemos atraer algun arma
		_elementPulled = checkPullItem();

		//si no podemos atraer nada, no hacemos nada
		if(!_elementPulled)
			return;
		
		//cogemos la entidad estatica y la desactivamos
		_elementPulled->deactivate();
		CGameNetMsgManager::getSingletonPtr()->sendDeactivateEntity(_elementPulled->getEntityID());
		//nos creamos una nueva entidad como la que hemos cogido pero dinamica,
		//para ello cogemos la informacion basica de la entidad dinamica y la
		//rellenamos con la informaci�n de la entidad que estamos creando
		Map::CEntity* info = new Map::CEntity(*CEntityFactory::getSingletonPtr()->getInfo(_elementPulled->getType()+"Dynamic"));

		std::string weapon, mesh, reward;

		auto componentCSpawnItemManager= _elementPulled->getComponent<CSpawnItemManager>("CSpawnItemManager");
		std::stringstream weaponaux ;
		weaponaux <<  componentCSpawnItemManager->getWeaponType();
		weapon = weaponaux.str();

		std::stringstream rewardaux;
		rewardaux <<  componentCSpawnItemManager->getReward();
		reward = rewardaux.str();

		//info->setAttribute("model",_elementPulled->getComponent<CGraphics>("CGraphics")->getMeshName());
		info->setAttribute("weaponType", weapon);
		info->setAttribute("reward", reward);
		info->setAttribute("id", componentCSpawnItemManager->getId());
		
		Map::CEntity* clientEntityInfo = new Map::CEntity( info->getName() );
		clientEntityInfo->setAttribute("model",componentCSpawnItemManager->getModel());

		//creamos la entidad con la informaci�n obtenida
		CEntity * dynamicItem = CEntityFactory::getSingletonPtr()->createCustomClientEntity(info,
																				clientEntityInfo,
																				Logic::CServer::getSingletonPtr()->getMap(),
																				_elementPulled->getPosition(),
																				_elementPulled->getOrientation());

		if(!dynamicItem){
			std::cout << "ALGO HA IDO MAL Y NO SE HA CREADO LA ENTIDAD" << std::endl;
			return;
		}
		
		_elementPulling = dynamicItem;

		//por ultimo, ponemos a la entidad donde debe estar
		_elementPulling->getComponent<CPhysicDynamicEntity>("CPhysicDynamicEntity")->setTransform(_elementPulled->getPosition(),_elementPulled->getOrientation(), true);

		//le metemos donde estamos para que nos siga
		_elementPulling->getComponent<CPullingMovement>("CPullingMovement")->setPlayer(_entity);

		_elementPulling->activate();
		_elementPulling->start();

		delete info;
	} // secondaryShoot
	//__________________________________________________________________

	CEntity * CSoulReaper::checkPullItem() {
		//El origen debe ser m�nimo la capsula (si chocamos el disparo en la capsula al mirar en diferentes direcciones ya esta tratado en la funcion de colision)
		//Posicion de la entidad + altura de disparo(coincidente con la altura de la camara)
		Vector3 origin = _entity->getPosition()+Vector3(0.0f,_heightShoot,0.0f);
		// Creamos el ray desde el origen en la direccion del raton (desvio ya aplicado)
		Ray ray(origin, (_entity->getOrientation()*Vector3::NEGATIVE_UNIT_Z).normalisedCopy());

		// Rayo lanzado por el servidor de f�sicas de acuerdo a la distancia de potencia del arma
		Physics::CRaycastHit hit;
		int nbHits = 0;
		//drawRaycast(ray);
		
		bool valid = Physics::CServer::getSingletonPtr()->raycastSingle(ray, _shotsDistanceSecondaryFire, hit,Physics::CollisionGroup::eITEM);
		
		if(!valid)
			return NULL;
		CEntity *entity = hit.entity;

		if(entity->getType()!="ItemSpawn")
			return NULL;

		return entity;
	} // checkPullItem
	//__________________________________________________________________

	void CSoulReaper::stopSecondaryFire(){
		//si cuando hice click no cogi nada no puedo hacer nada aqui
		if(!_elementPulling)
			return;
		CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_elementPulling, true);
		_elementPulled->activate();
		CGameNetMsgManager::getSingletonPtr()->sendActivateEntity(_elementPulled->getEntityID());
	} // stopSecondaryFire
	//__________________________________________________________________

	void CSoulReaper::resetEntityPulling(){
		_elementPulled->activate();
		CGameNetMsgManager::getSingletonPtr()->sendActivateEntity(_elementPulled->getEntityID());
		_elementPulled->getComponent<CSpawnItemManager>("CSpawnItemManager")->beginRespawn();
		std::shared_ptr<CMessageActivate> deactivateMsg = std::make_shared<CMessageActivate>();
		deactivateMsg->setActivated(false);
		_elementPulled->emitMessage(deactivateMsg);

		_elementPulling = NULL;

		//vamos a decirle al spawnitem original que le han cogido, diciendole
		//que comience su periodo de respawn y desactivando su fisica
		_elementPulled->getComponent<CSpawnItemManager>("CSpawnItemManager")->beginRespawn();
		_elementPulled->getComponent<CPhysicStaticEntity>("CPhysicStaticEntity")->deactivateSimulation();

		//nos liberamos del puntero
		_elementPulled=NULL;
	} // resetEntityPulling
	//__________________________________________________________________

	void CSoulReaper::amplifyDamage(unsigned int percentage) {
		// Si es 0 significa que hay que restaurar al que habia por defecto
		if(percentage == 0) {
			_primaryFireDamage = _defaultPrimaryFireDamage;
		}
		// Sino aplicamos el porcentaje pasado por par�metro
		else {
			_primaryFireDamage += percentage * _primaryFireDamage * 0.01f;
		}
	} // amplifyDamage
	//__________________________________________________________________

	void CSoulReaper::onTick(unsigned int msecs) {
		/*
		// Controlamos el cooldown del disparo primario y secundario
		if(_primaryFireTimer > 0) {
			_primaryFireTimer -= msecs;
			
			if(_primaryFireTimer < 0){
				_primaryFireTimer = 0;
				
			}

		}
		*/
	} // onTick
	//__________________________________________________________________
} // namespace Logic

