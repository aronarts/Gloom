/**
@file CharacterName.cpp


@see Logic::CCharacterName
@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Agosto, 2013
*/

#include "CharacterName.h"
#include "Graphics.h"
#include "AnimatedGraphics.h"

#include "Graphics/Movable2dText.h"

#include "Application/BaseApplication.h"
#include "Map/MapEntity.h"
#include "Logic/Maps/Map.h"

using namespace std;

namespace Logic {

	IMP_FACTORY(CCharacterName);

	//________________________________________________________________________

	CCharacterName::CCharacterName() : _text2d(NULL),
									   _color(1.0f, 1.0f, 1.0f) {
		// Nada que hacer
	}

	//________________________________________________________________________

	CCharacterName::~CCharacterName() {
		if(_text2d != NULL) {
			delete _text2d;
		}
	}
	
	//________________________________________________________________________
	
	bool CCharacterName::spawn(CEntity* entity, CMap* map, const Map::CEntity* entityInfo) {
		if( !IComponent::spawn(entity,map,entityInfo) ) return false;

		assert( entityInfo->hasAttribute("textOffset") );
		assert( entityInfo->hasAttribute("textFont") );

		_offset = entityInfo->getVector3Attribute("textOffset");
		_font = entityInfo->getStringAttribute("textFont");

		if( entityInfo->hasAttribute("textColor") )
			_color = entityInfo->getVector3Attribute("textColor");

		return true;
	} // spawn

	//________________________________________________________________________

	void CCharacterName::setVisible(bool isVisible) {
		_text2d->setVisible(isVisible);
	}

	//________________________________________________________________________

	bool CCharacterName::isVisible() {
		return _text2d->isVisible();
	}

	//________________________________________________________________________

	void CCharacterName::onStart() {
		_text2d = new Graphics::CMovable2dText();

		CGraphics* graphicComponent = _entity->getComponent<CAnimatedGraphics>("CAnimatedGraphics");
		if(graphicComponent != NULL) {
			Graphics::CEntity* graphicEntity = graphicComponent->getGraphicEntity();
			_text2d->load( _entity->getMap()->getScene(), graphicEntity, _offset, _entity->getName(), _font, Vector4(_color.x, _color.y, _color.z, 1.0f) );
		}
		else {
			_text2d->load( _entity->getMap()->getScene(), NULL, _offset, _entity->getName(), _font, Vector4(_color.x, _color.y, _color.z, 1.0f) );
		}
	}

} // namespace Logic
