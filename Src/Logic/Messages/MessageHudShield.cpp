#include "MessageHudShield.h"

#include "Logic/Entity/MessageFactory.h"

namespace Logic {

	IMP_FACTORYMESSAGE(CMessageHudShield);

	CMessageHudShield::CMessageHudShield() : CMessage(Message::HUD_SHIELD) {
		// Nada que hacer
	}//
	//----------------------------------------------------------

	void CMessageHudShield::setShield( int shield){
		_shield = shield;
	}//
	//----------------------------------------------------------

	int CMessageHudShield::getShield(){
		return _shield;
	}//
	//----------------------------------------------------------
		
	Net::CBuffer* CMessageHudShield::serialize() {
		assert(_tempBuffer == NULL);

		_tempBuffer = new Net::CBuffer(sizeof(int) + sizeof(_shield));
		_tempBuffer->serialize(std::string("CMessageHudShield"),true);
		_tempBuffer->serialize(_shield);
		
		return _tempBuffer;
	}//
	//----------------------------------------------------------

	void CMessageHudShield::deserialize(Net::CBuffer& buffer) {
		buffer.deserialize(_shield);
	}

};
