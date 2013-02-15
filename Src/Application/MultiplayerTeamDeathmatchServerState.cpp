//---------------------------------------------------------------------------
// MultiplayerTeamDeathmatchServerState.cpp
//---------------------------------------------------------------------------

/**
@file MultiplayerTeamDeathmatchServerState.cpp

Contiene la implementaci�n del estado de juego.

@see Application::MultiplayerTeamDeathmatchServerState
@see Application::CGameState

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#include "MultiplayerTeamDeathmatchServerState.h"

#include "Net/Manager.h"
#include "Net/Servidor.h"
#include "Net/factoriared.h"
#include "Net/paquete.h"
#include "Net/buffer.h"

#include "Logic/GameNetPlayersManager.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Server.h"
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/Map.h"
#include "Logic/Maps/EntityID.h"

namespace Application {


	void CMultiplayerTeamDeathmatchServerState::activate() 
	{
		CGameState::activate();

		Net::CManager::getSingletonPtr()->addObserver(this);
	} // activate

	//--------------------------------------------------------

	void CMultiplayerTeamDeathmatchServerState::deactivate() 
	{
		Net::CManager::getSingletonPtr()->removeObserver(this);
		Net::CManager::getSingletonPtr()->deactivateNetwork();

		CGameState::deactivate();
	} // deactivate

	//--------------------------------------------------------
	
	void CMultiplayerTeamDeathmatchServerState::dataPacketReceived(Net::CPaquete* packet)
	{
		// Obtenemos la id de la conexion por la que hemos recibido el paquete (para identificar
		// al cliente)
		Net::NetID playerNetId = packet->getConexion()->getId();

		Net::CBuffer buffer(packet->getDataLength());
		buffer.write(packet->getData(), packet->getDataLength());
		buffer.reset(); // Desplazamos el puntero al principio para realizar la lectura

		Net::NetMessageType msg;
		buffer.read(&msg, sizeof(msg));
		
		switch (msg)
		{
		case Net::PLAYER_INFO:
		{
			std::string playerNick, playerMesh;
			buffer.deserialize(playerNick);
			buffer.deserialize(playerMesh);

			Logic::CGameNetPlayersManager::getSingletonPtr()->setPlayerNickname(playerNetId, playerNick);
			Logic::CGameNetPlayersManager::getSingletonPtr()->setPlayerMesh(playerNetId, playerMesh);

			// Una vez recibida la informacion del cliente, le indicamos que cargue el mapa (solo
			// a ese cliente concreto)
			Net::NetMessageType msg = Net::LOAD_MAP;
			Net::CBuffer buffer(sizeof(msg));
			buffer.write(&msg, sizeof(msg));

			Net::CManager::getSingletonPtr()->send(buffer.getbuffer(), buffer.getSize(), playerNetId);

			break;
		}
		case Net::MAP_LOADED:
		{
			// EN ESTA FASE EL CLIENTE PASA PASA A CARGAR LOS CLIENTES DE LA PARTIDA Y VICEVERSA,
			// LOS CLIENTES DE LA PARTIDA PASAN A CARGAR EL NUEVO CLIENTE

			// Variables locales
			Net::CBuffer tempBuffer;
			std::string name;
			Logic::TEntityID entityId;
			Net::NetID netId;
			Net::NetMessageType msg = Net::LOAD_PLAYER;

			// Mandamos la informacion de los players de la partida al cliente que esta intentando conectarse
			Logic::CGameNetPlayersManager::iterator playerInfoIt = Logic::CGameNetPlayersManager::getSingletonPtr()->begin();
			for(; playerInfoIt != Logic::CGameNetPlayersManager::getSingletonPtr()->end(); ++playerInfoIt) {
				netId = playerInfoIt->getNetId();

				// Debido a que fuera de este bucle enviaremos la informacion de este player mediante broadcast
				// evitamos enviar la informacion en esta fase (ya que la id de entidad aun no ha sido asignada)
				if(netId != playerNetId) {
					entityId = playerInfoIt->getEntityId();
					name = playerInfoIt->getName();

					// Mensaje LOAD_PLAYER
					tempBuffer.write(&msg, sizeof(msg));
					tempBuffer.write(&netId, sizeof(netId));
					tempBuffer.write(&entityId, sizeof(entityId));
					tempBuffer.serialize(name, false);

					// Enviamos los datos de los clientes conectados al cliente que se quiere conectar
					Net::CManager::getSingletonPtr()->send(buffer.getbuffer(), buffer.getSize(), playerNetId);

					// Reseteamos el puntero de escritura del buffer para escribir en la siguiente vuelta del bucle
					tempBuffer.reset();
				}
			}

			Logic::CPlayerInfo playerInfo = Logic::CGameNetPlayersManager::getSingletonPtr()->getPlayer(playerNetId);
			name = playerInfo.getName();

			// Creamos un player en el mundo con el nombre del jugador que solicita entrar
			Logic::CEntity* player = Logic::CServer::getSingletonPtr()->getMap()->createPlayer(name);
			// Extraemos la id asignada a dicha entidad y la asociamos al player del gestor
			entityId = player->getEntityID();
			Logic::CGameNetPlayersManager::getSingletonPtr()->setEntityID(playerNetId, entityId);
			
			// Ordenamos la carga del player a todos los clientes conectados, evidentemente el propio cliente que se quiere
			// conectar tendra que obviar ignorar este mensaje (comprobando que el cliente que se carga es �l mismo).

			// El contenido de msg es LOAD_PLAYER
			Net::CBuffer buffer( sizeof(msg) + sizeof(playerNetId) + sizeof(entityId) + (sizeof(char) * name.size()) );
			buffer.write(&msg, sizeof(msg));
			buffer.write(&playerNetId, sizeof(playerNetId));
			buffer.write(&entityId, sizeof(entityId));
			buffer.serialize(name, false);

			// Broadcast a todos los jugadores del id de este jugador
			Net::CManager::getSingletonPtr()->send(buffer.getbuffer(), buffer.getSize());

			break;
		}
		case Net::PLAYER_LOADED:
		{
			//Aumentamos el n�mero de jugadores cargados por el cliente
			Net::NetID playerLoadedNetId;
			buffer.read(&playerLoadedNetId, sizeof(playerLoadedNetId));
			Logic::CGameNetPlayersManager::getSingletonPtr()->loadPlayer( packet->getConexion()->getId(), playerLoadedNetId );

			// MANDAR EL MENSAJE DE ARRANQUE A TAN SOLO UNO DE ELLOS (EL QUE SE QUIERE CONECTAR)

			// Problema, si se conecta alguien antes de llegar a este if, el numero de jugadores conectados
			// incrementa (cosa que se hace cuando se recibe un paquete de conexion) y ya no se cumpliria
			// el if

			//@deprecated
			//Si todos los clientes han cargado todos los players
			if( Logic::CGameNetPlayersManager::getSingletonPtr()->getPlayersLoaded(playerNetId) == 
				Logic::CGameNetPlayersManager::getSingletonPtr()->getNumberOfPlayersConnected() ) {
				
				// Si el cliente que queria conectarse a cargado a todos los players mandamos el mensaje
				// de empezar
				Net::NetMessageType msg = Net::START_GAME;
				Net::CManager::getSingletonPtr()->send(&msg, sizeof(msg), playerNetId);
			}

			break;
		}
		}

	} // dataPacketReceived

	//--------------------------------------------------------

	void CMultiplayerTeamDeathmatchServerState::connexionPacketReceived(Net::CPaquete* packet)
	{
		Net::NetID playerId = packet->getConexion()->getId();

		// @TODO
		// Garantizar que todos los clientes se conectan en la fase adecuada de manera que no se
		// produzcan condiciones indeseables a causa de la concurrencia
		
		// Actualizamos el manager de jugadores
		Logic::CGameNetPlayersManager::getSingletonPtr()->addPlayer(playerId);

		// Avisamos (solo) al player de que comience a enviarnos sus datos
		Net::NetMessageType msg = Net::LOAD_PLAYER_INFO;
		Net::CBuffer buffer(sizeof(msg));
		buffer.write(&msg, sizeof(msg));

		Net::CManager::getSingletonPtr()->send(buffer.getbuffer(), buffer.getSize(), playerId);
	} // connexionPacketReceived

	//--------------------------------------------------------

	void CMultiplayerTeamDeathmatchServerState::disconnexionPacketReceived(Net::CPaquete* packet)
	{
		// @todo
		// NOTIFICAR A LOS CLIENTES PARA QUE ELIMINEN ESE JUGADOR DE LA PARTIDA

		// Actualizamos el manager de jugadores
		Logic::CGameNetPlayersManager::getSingletonPtr()->removePlayer( packet->getConexion()->getId() );
	} // disconnexionPacketReceived
	
} // namespace Application
