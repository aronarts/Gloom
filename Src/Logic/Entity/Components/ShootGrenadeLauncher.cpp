/**
@file ShootGrenadeLauncher.cpp

Contiene la implementacion del componente
de disparo del lanzagranadas.

@see Logic::CShootGrenadeLauncher
@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#include "ShootGrenadeLauncher.h"

#include "Logic/Maps/EntityFactory.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Server.h"

#include "Logic/Messages/MessageSetPhysicPosition.h"
#include "Logic/Messages/MessageAddForcePhysics.h"

namespace Logic {
	IMP_FACTORY(CShootGrenadeLauncher);

	bool CShootGrenadeLauncher::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		if(!CShootProjectile::spawn(entity, map, entityInfo)) return false;

		// Leer los parametros que toquen para los proyectiles
		std::stringstream aux;
		aux << "weapon" << _nameWeapon;	////!!!! Aqui debes de poner el nombre del arma que leera en el map.txt
		std::string weapon = aux.str();

		_shootForce = entityInfo->getFloatAttribute(weapon+"ShootForce");

		return true;
	}

	//__________________________________________________________________

	void CShootGrenadeLauncher::fireWeapon() {
		// Obtenemos la informacion asociada al arquetipo de la granada
		Map::CEntity *entityInfo = CEntityFactory::getSingletonPtr()->getInfo("Grenade");
		// Creamos la entidad y la activamos
		CEntity* grenade = CEntityFactory::getSingletonPtr()->createEntity( entityInfo, Logic::CServer::getSingletonPtr()->getMap() );
		grenade->activate();

		// Spawneamos la granada justo delante del jugador y a la altura de disparo que corresponda
		Vector3 myPosition = _entity->getPosition() + ( Math::getDirection( _entity->getOrientation() ) * (_capsuleRadius) );
		myPosition.y = _heightShoot - _projectileRadius;

		// Mensaje para situar el collider fisico de la granada en la posicion de disparo.
		Logic::CMessageSetPhysicPosition* msg = new Logic::CMessageSetPhysicPosition();
		msg->setPosition(myPosition);
		grenade->emitMessage(msg);
		
		// Mandar mensaje add force
		Logic::CMessageAddForcePhysics* forceMsg = new Logic::CMessageAddForcePhysics();
		forceMsg->setForceVector( Math::getDirection( _entity->getOrientation()) * _shootForce );
		grenade->emitMessage(forceMsg);
	}
	
} // namespace Logic

