
/**
@file PositionInterpolator.cpp
 
@see Logic::CPositionInterpolator
@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#include "PositionInterpolator.h"
#include "PhysicController.h"

// Map
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"

// Mensajes
#include "Logic/Messages/Message.h"
#include "Logic/Messages/MessagePositionSnapshot.h"

using namespace std;

namespace Logic {
	
	IMP_FACTORY(CPositionInterpolator);

	//__________________________________________________________________

	CPositionInterpolator::CPositionInterpolator() : _lostTicks(0) {
		// Nada que hacer
	}

	//__________________________________________________________________

	CPositionInterpolator::~CPositionInterpolator() {
		// Nada que hacer
	}

	//__________________________________________________________________

	bool CPositionInterpolator::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IComponent::spawn(entity, map, entityInfo) ) return false;

		assert( entityInfo->hasAttribute("ticksPerSample") );

		// Cada cuantos fixed ticks tomamos una muestra
		_ticksPerSample = entityInfo->getIntAttribute("ticksPerSample");

		return true;
	}

	//__________________________________________________________________

	bool CPositionInterpolator::accept(const shared_ptr<CMessage>& message) {
		return message->getMessageType() == Message::POSITION_SNAPSHOT;
	}

	//__________________________________________________________________

	void CPositionInterpolator::interpolateSnapshot(const vector<Vector3>& buffer) {
		Vector3 inc, temp;
		unsigned int bufferSize = buffer.size();

		// Interpolar la primera posicion
		if(!_buffer.empty()) {
			unsigned int lastIndex = _buffer.size() - 1;
			inc = (buffer[0] - _buffer[lastIndex]) / (float)_ticksPerSample;

			temp = _buffer[lastIndex] + inc;
			for(int k = 0; k < _ticksPerSample - 1; ++k) {
				_buffer.push_back(temp);
				temp += inc;
			}
		}

		for(int i = 0; i < bufferSize - 1; ++i) {
			_buffer.push_back(buffer[i]);

			inc = (buffer[i+1] - buffer[i]) / (float)_ticksPerSample;

			temp = buffer[i] + inc;
			for(int k = 0; k < _ticksPerSample - 1; ++k) {
				_buffer.push_back(temp);
				temp += inc;
			}
		}

		_buffer.push_back(buffer[bufferSize - 1]);
	}

	//__________________________________________________________________

	void CPositionInterpolator::process(const shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::POSITION_SNAPSHOT: {
				vector<Vector3> buffer = static_pointer_cast<CMessagePositionSnapshot>(message)->getBuffer();

				interpolateSnapshot(buffer);

				break;
			}
		}
	}

	//__________________________________________________________________

	void CPositionInterpolator::onStart() {
		_controller = _entity->getComponent<CPhysicController>("CPhysicController");
		assert(_controller != NULL && "Error: Como crees que voy a interpolar si no tengo physicController? meh!");
	}

	//__________________________________________________________________

	void CPositionInterpolator::onFixedTick(unsigned int msecs) {
		if( !_buffer.empty() ) {
			_controller->setPhysicPosition(_buffer.front());
			_buffer.pop_front();
		}
		// Estamos perdiendo ticks por algun motivo
		// hay que extrapolar y descartar del buffer
		// que recibamos las que hemos perdido
		else {
			//cout << "Perdiendo snapshots" << endl;
		}
	}

} // namespace Logic


