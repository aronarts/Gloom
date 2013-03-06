/**
@file Light.cpp

Contiene la implementación del componente que controla la vida de una entidad.
 
@see Logic::CLight
@see Logic::IComponent

@author Antonio Jesus Narvaez
@date Marzo, 2013
*/

#include "Light.h"

#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"
#include "Application/BaseApplication.h"
#include "Graphics/Light.h"

#include "Logic/Server.h"

namespace Logic 
{
	IMP_FACTORY(CLight);
	
	//---------------------------------------------------------
	
	bool CLight::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;

		_light = new Graphics::CLight();
		Vector3 position = entityInfo->getVector3Attribute("position");
		Vector3 direction;

		if(entityInfo->hasAttribute("direction"))
			direction = entityInfo->getVector3Attribute("direction");

		std::string type = entityInfo->getStringAttribute("type");
		if(type == "SpotLight")
			_light->createSpotlLight(_entity->getName(), position, direction);
		if(type == "DirectionalLight")
			_light->createDirectionalLight(_entity->getName(), position, direction);
		if(type == "PointLight")
			_light->createPointLight(_entity->getName(), position);

		if(entityInfo->hasAttribute("castShadows"))
			_light->setCastShadows(entityInfo->getBoolAttribute("castShadows"));
		
		if(entityInfo->hasAttribute("colour")){
			Vector3 colour = entityInfo->getVector3Attribute("colour");
			_light->setColour(colour.x, colour.y, colour.z);
		}
		
		if(entityInfo->hasAttribute("specularColour")){
			Vector3 specularColour = entityInfo->getVector3Attribute("specularColour");
			_light->setColour(specularColour.x, specularColour.y, specularColour.z);
		}

		if(entityInfo->hasAttribute("intensity"))
			_light->setIntensity(entityInfo->getFloatAttribute("intensity"));

		_light->attachToScene();

		return true;

	} // spawn
	
	//---------------------------------------------------------


	
	void CLight::activate()
	{
		IComponent::activate();

	} // activate
	//---------------------------------------------------------



	bool CLight::accept(CMessage *message)
	{
		return false;
		//return message->getMessageType() == Message::DAMAGED;
		
	} // accept
	
	//---------------------------------------------------------

	void CLight::process(CMessage *message)
	{
		/*
		switch(message->getMessageType())
		{

		}
		*/

	} // process
	//----------------------------------------------------------
/*
	void CLight::tick(unsigned int msecs)
	{
		IComponent::tick(msecs);

		
	} // tick
	//----------------------------------------------------------
*/


} // namespace Logic


