/**
@file Screamer.cpp

Contiene la implementaci�n de la clase que
implementa las habilidades del personaje
"Screamer" (servidor).
 
@see Logic::CScreamerServer
@see Logic::IComponent

@author Francisco Aisa Garc�a.
@date Marzo, 2013
*/

// Componentes
#include "ScreamerServer.h"
#include "ScreamerShieldDamageNotifier.h"
#include "PhysicDynamicEntity.h"
#include "Graphics.h"
#include "Life.h"
#include "Logic/Maps/WorldState.h"

// Logica
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Server.h"
#include "Logic/Messages/MessageSetPhysicPosition.h"
#include "Logic/Messages/MessageAddForcePlayer.h"
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageChangeMaterial.h"
#include "Logic/Messages/MessageActivateScreamerShield.h"
#include "Logic/Messages/MessageDeactivateScreamerShield.h"
#include "Logic/Messages/MessageSetRelatedEntity.h"

// F�sica
#include "Physics/Server.h"
#include "Physics/GeometryFactory.h"

// Gr�ficos
#include "Graphics/Server.h"
#include "Graphics/Particle.h"
#include "Graphics/Scene.h"

#include "Graphics/Camera.h"

namespace Logic {

	IMP_FACTORY(CScreamerServer);

	//__________________________________________________________________

	CScreamerServer::CScreamerServer() : CPlayerClass("screamer"),
										_primarySkillIsActive(false),
										_screamerShieldDamageTimer(0),
										_screamerShieldRecoveryTimer(0),
										_screamerShield(NULL) {
	}

	//__________________________________________________________________

	CScreamerServer::~CScreamerServer() {
		// Nada que hacer
	}
	
	//__________________________________________________________________

	bool CScreamerServer::spawn(CEntity* entity, CMap* map, const Map::CEntity* entityInfo) {
		if( !CPlayerClass::spawn(entity,map,entityInfo) ) return false;

		if(entityInfo->hasAttribute("materialName"))
			_materialName = entityInfo->getStringAttribute("materialName");

		assert( entityInfo->hasAttribute("screamerShieldThreshold") );
		assert( entityInfo->hasAttribute("screamerShieldDamageTimeStep") );
		assert( entityInfo->hasAttribute("screamerShieldDamageOverTime") );
		assert( entityInfo->hasAttribute("screamerShieldRecoveryTimeStep") );
		assert( entityInfo->hasAttribute("screamerShieldRecoveryOverTime") );
		assert( entityInfo->hasAttribute("screamerExplotionDamage") );
		assert( entityInfo->hasAttribute("screamerExplotionRadius") );

		_currentScreamerShield = _screamerShieldThreshold = entityInfo->getFloatAttribute("screamerShieldThreshold");
		_screamerShieldDamageTimeStep = entityInfo->getFloatAttribute("screamerShieldDamageTimeStep");
		_screamerShieldDamageOverTime = entityInfo->getFloatAttribute("screamerShieldDamageOverTime");
		_screamerShieldRecoveryTimeStep = entityInfo->getFloatAttribute("screamerShieldRecoveryTimeStep");
		_screamerShieldRecoveryOverTime = entityInfo->getFloatAttribute("screamerShieldRecoveryOverTime");
		_screamerExplotionDamage = entityInfo->getFloatAttribute("screamerExplotionDamage");
		_screamerExplotionRadius = entityInfo->getFloatAttribute("screamerExplotionRadius");
		return true;
	} // spawn

	//__________________________________________________________________

	void CScreamerServer::onTick(unsigned int msecs) {
		CPlayerClass::onTick(msecs);

		// Si la habilidad primaria est� siendo usada
		if(_primarySkillIsActive) {
			// Deberia funcionar este bucle como el de la fisica? restando
			// da�os hasta cumplir los ciclos acumulados?
			if(_screamerShieldDamageTimer < _screamerShieldDamageTimeStep) {
				_screamerShieldDamageTimer += msecs;
			}
			else {
				_currentScreamerShield -= _screamerShieldDamageOverTime;
				_screamerShieldDamageTimer = 0;

				if(_currentScreamerShield > 0) {
					// Colocamos el escudo delante nuestro
					refreshShieldPosition();
				}
				else {
					_primarySkillIsActive = false;

					// Destruir o desactivar el escudo
					deactivateScreamerShield();

					// Crear explosi�n
					createExplotion();

					// Matar a mi propia entidad
					CLife* lifeComponent = _entity->getComponent<CLife>("CLife");
					assert(lifeComponent && "Error: El jugador no tiene un componente CLife");
					lifeComponent->suicide();
				}
			}
		}
		else if(_currentScreamerShield != _screamerShieldThreshold) {
			if(_screamerShieldRecoveryTimer < _screamerShieldRecoveryTimeStep) {
				_screamerShieldRecoveryTimer += msecs;
			}
			else {
				_currentScreamerShield += _screamerShieldRecoveryOverTime;
				_screamerShieldRecoveryTimer = 0;

				if(_currentScreamerShield > _screamerShieldThreshold) {
					_currentScreamerShield = _screamerShieldThreshold;
				}
			}
		}
	}

	//________________________________________________________________________

	void CScreamerServer::onActivate() {
		CPlayerClass::onActivate();

		_primarySkillIsActive = false;
		_currentScreamerShield = _screamerShieldThreshold;
		_screamerShieldDamageTimer = _screamerShieldRecoveryTimer = 0;
	}

	//__________________________________________________________________

	void CScreamerServer::primarySkill() {

		_primarySkillIsActive = true;

		// @deprecated DE MOMENTO HAGO ESTA COMPROBACI�N �APERA
		// porque el cliente no recibe el mensaje si lo hacemos en el onStart
		// otra posibilidad es crear el escudo cada vez que pulsemos la tecla.
		if(_screamerShield == NULL) {
			// Nada que hacer
			// Obtenemos la informacion asociada al arquetipo de la granada
			Map::CEntity* screamerShieldInfo = CEntityFactory::getSingletonPtr()->getInfo("ScreamerShield");

			// Creamos la entidad y la activamos
			_screamerShield = CEntityFactory::getSingletonPtr()->createEntity( screamerShieldInfo, Logic::CServer::getSingletonPtr()->getMap() );
			assert(_screamerShield != NULL);

			std::shared_ptr<CMessageSetRelatedEntity> relatedEntityMsg = std::make_shared<CMessageSetRelatedEntity>();
			relatedEntityMsg->setRelatedEntity(_screamerShield);
			_entity->emitMessage(relatedEntityMsg);

			// Fijamos a nuestra entidad como due�a de la entidad creada en el componente
			// que recibe las notificaciones de da�o.
			CScreamerShieldDamageNotifier* shieldDmgNotifier = _screamerShield->getComponent<CScreamerShieldDamageNotifier>("CScreamerShieldDamageNotifier");
			assert(shieldDmgNotifier && "Error: La entidad ScreamerShield no tiene ningun componente llamado CScreamerShieldDamageNotifier");
			shieldDmgNotifier->setOwner(_entity);

			// @deprecated El servidor no va a tener graficos
			CGraphics* shieldGraphics = _screamerShield->getComponent<CGraphics>("CGraphics");
			assert(shieldGraphics && "Error: La entidad ScreamerShield no tiene un componente CGraphics");
			shieldGraphics->setVisible(false);
			_screamerShield->deactivate();
		}

		// Activamos los gr�ficos y la entidad
		activateScreamerShield();
	}

	//__________________________________________________________________

	void CScreamerServer::secondarySkill() {
		// Habilidad por definir


		std::cout << "Secondary Skill - Screamer" << std::endl;
	}

	//__________________________________________________________________

	void CScreamerServer::stopPrimarySkill() {
		_primarySkillIsActive = false;
		
		// Desactivamos los gr�ficos y la entidad
		deactivateScreamerShield();
	}

	//__________________________________________________________________

	void CScreamerServer::absorbDamage(float damage) {
		_currentScreamerShield -= damage;
	}

	//__________________________________________________________________

	void CScreamerServer::refreshShieldPosition() {
		// Sacamos la posicion del escudo (que debe estar situada a la altura de disparo)
		Vector3 shootPosition = _entity->getPosition() + ( Math::getDirection( _entity->getOrientation() ) * _capsuleRadius );
		shootPosition.y += _heightShoot;
		
		// Sacamos la orientacion de la entidad para setearsela al escudo
		Matrix4 shootTransform;
		shootTransform.setTrans(shootPosition);
		//Math::setPitchYaw( _entity->getPitch(), _entity->getYaw() + Math::HALF_PI, shootTransform);
		Math::setPitchYaw( _entity->getPitch(), _entity->getYaw(), shootTransform);

		// Posicionamos el centro del escudo justo en el punto de mira		
		CPhysicDynamicEntity* physicDynamic = _screamerShield->getComponent<CPhysicDynamicEntity>("CPhysicDynamicEntity");
		assert(physicDynamic && "Error la entidad ScreamerShield no tiene un componente fisico");
		physicDynamic->setTransform(shootTransform, false);
	}

	//__________________________________________________________________

	void CScreamerServer::activateScreamerShield() {
		// Mandamos el mensaje de activacion para la red
		_entity->emitMessage( std::make_shared<CMessageActivateScreamerShield>() );

		// Activamos el escudo
		_screamerShield->activate();
		// Activamos los gr�ficos del escudo
		CGraphics* shieldGraphics = _screamerShield->getComponent<CGraphics>("CGraphics");
		assert(shieldGraphics && "Error: La entidad ScreamerShield no tiene un componente CGraphics");
		shieldGraphics->setVisible(true);
	}

	//__________________________________________________________________

	void CScreamerServer::deactivateScreamerShield() {
		// Mandamos el mensaje de desactivacion para la red
		_entity->emitMessage( std::make_shared<CMessageDeactivateScreamerShield>() );
		CGraphics* shieldGraphics = _screamerShield->getComponent<CGraphics>("CGraphics");
		assert(shieldGraphics && "Error: La entidad ScreamerShield no tiene un componente CGraphics");
		shieldGraphics->setVisible(false);
		_screamerShield->deactivate();
	}

	//__________________________________________________________________

	void CScreamerServer::createExplotion() {
		// EntitiesHit sera el buffer que contendra la lista de entidades que ha colisionado
		// con el overlap
		std::vector<CEntity*> entitiesHit;

		// Hacemos una query de overlap con la geometria de una esfera en la posicion 
		// en la que se encuentra la granada con el radio que se indique de explosion
		Physics::SphereGeometry explotionGeom = Physics::CGeometryFactory::getSingletonPtr()->createSphere(_screamerExplotionRadius);
		Physics::CServer::getSingletonPtr()->overlapMultiple(explotionGeom, _entity->getPosition(), entitiesHit);

		int nbHits = entitiesHit.size();
		// Mandamos el mensaje de da�o a cada una de las entidades que hayamos golpeado
		// Adem�s aplicamos un desplazamiento al jugador 
		for(int i = 0; i < nbHits; ++i) {
			// Si la entidad golpeada es valida
			if(entitiesHit[i] != NULL) {
				// Emitimos el mensaje de da�o
				std::shared_ptr<CMessageDamaged> dmgMsg = std::make_shared<CMessageDamaged>();
				dmgMsg->setDamage(_screamerExplotionDamage);
				dmgMsg->setEnemy(_entity);
				entitiesHit[i]->emitMessage(dmgMsg);
				
				// Emitimos el mensaje de desplazamiento por da�os
				
				/*				
				std::shared_ptr<CMessageAddForcePlayer> forceMsg = std::make_shared<CMessageAddForcePlayer>();

				// Seteamos la fuerza y la velocidad
				forceMsg->setPower(0.1f);
				forceMsg->setVelocity(0.12f);
				// Seteamos el vector director del desplazamiento
				Vector3 impulseDirection = entitiesHit[i]->getPosition() - _entity->getPosition();
				impulseDirection.normalise();
				forceMsg->setDirection(impulseDirection);
				entitiesHit[i]->emitMessage(forceMsg);*/
			}
		}

		Graphics::CParticle *particle = Graphics::CServer::getSingletonPtr()->
			getActiveScene()->createParticle(_entity->getName(),"ExplosionParticle", _entity->getPosition());

	}

} // namespace Logic

