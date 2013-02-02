#ifndef __Logic_MessageSetAnimation_H
#define __Logic_MessageSetAnimation_H

#include "Message.h"

namespace Logic {

	class CMessageSetAnimation : public CMessage{
	public:
		CMessageSetAnimation(TMessageType t);
		std::string getString();
		void setString(std::string);
		bool getBool();
		void setBool(bool boolean);
		~CMessageSetAnimation(){};

		virtual Net::CBuffer* serialize();

	private:
		std::string _string;
		bool _bool;
	};

};

#endif