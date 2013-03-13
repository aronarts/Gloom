/**
@file ClientRespawn.cpp

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#include "ClientRespawn.h"

#include "Logic/Entity/Entity.h"
#include "PhysicController.h"
#include "Logic/Server.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"
#include "Basesubsystems/Math.h"

#include "Logic/Messages/MessagePlayerDead.h"
#include "Logic/Messages/MessagePlayerSpawn.h"

#include <math.h>

#include <iostream>

namespace Logic  {
	
	IMP_FACTORY(CClientRespawn);

	//________________________________________________________________________

	bool CClientRespawn::accept(CMessage *message) {
		return message->getMessageType() == Message::PLAYER_DEAD ||
			   message->getMessageType() == Message::PLAYER_SPAWN;
		//return false;
	} // accept

	//________________________________________________________________________

	void CClientRespawn::process(CMessage *message) {
		switch(message->getMessageType()) {
		case Message::PLAYER_DEAD:
			{
			// El servidor nos notifica de que hemos muerto, desactivamos
			// la entidad al completo y su simulacion fisica para que no
			// podamos colisionar con la c�psula del jugador.

			// Desactivamos todos los componentes menos estos
			std::vector<std::string> exceptionList;
			exceptionList.push_back( std::string("CClientRespawn") );
			exceptionList.push_back( std::string("CHudOverlay") );
			exceptionList.push_back( std::string("CNetConnector") );
			//exceptionList.push_back( std::string("CAvatarController") );
			//exceptionList.push_back( std::string("CPhysicController") );

			// En caso de estar simulando fisica en el cliente, desactivamos
			// la c�psula.
			CPhysicController* controllerComponent = _entity->getComponent<CPhysicController>("CPhysicController");
			if(controllerComponent != NULL) {
				controllerComponent->deactivateSimulation();
			}

			_entity->deactivateAllComponentsExcept(exceptionList);
			
			break;
			}
		case Message::PLAYER_SPAWN:
			{
			// El servidor nos notifica de que debemos respawnear. Activamos
			// todos los componentes de la entidad y seteamos nuestra posicion
			// en el lugar indicado por el mensaje recibido del servidor.

			CMessagePlayerSpawn* playerSpawnMsg = static_cast<CMessagePlayerSpawn*>(message);

			// En caso de estar simulando fisica en el cliente, reactivamos las colisiones
			// y reposicionamos la capsula donde nos diga el servidor.
			CPhysicController* controllerComponent = _entity->getComponent<CPhysicController>("CPhysicController");
			if(controllerComponent != NULL) {
				controllerComponent->activateSimulation();
				controllerComponent->setPhysicPosition( playerSpawnMsg->getSpawnPosition() );
			}

			// Volvemos a activar todos los componentes
			//_entity->activate();
			
			break;
			}
		}
	} // process

} // namespace Logic
