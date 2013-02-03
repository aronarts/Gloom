/***

@see Logic::CHudOverlay
@see Logic::IComponent

@author Antonio Jesus
@date Febrero, 2013
*/
#ifndef __Logic_HudOverlay_H
#define __Logic_HudOverlay_H

#include "Logic/Entity/Component.h"

//declaraci�n de la clase
namespace Logic 
{
/**
	Este componente controla el hud, mediante overlays
    @ingroup logicGroup

	@author Antonio Jesus Narvaez
	@date Febrero, 2013
*/
	class CHudOverlay : public IComponent
	{
		DEC_FACTORY(CHudOverlay);
	public:

		/**
		Constructor por defecto; en la clase base no hace nada.
		*/
		CHudOverlay() : IComponent(), _health(0), _shield(0) {}
		
		/**
		Inicializaci�n del componente usando la descripci�n de la entidad que hay en 
		el fichero de mapa.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		/**
		Este componente s�lo acepta mensajes de tipo DAMAGED.
		*/
		virtual bool accept(CMessage *message);

		/**
		Al recibir un mensaje de tipo DAMAGED la vida de la entidad disminuye.
		*/
		virtual void process(CMessage *message);

		/**
		Al recibir da�o quitamos vida
		*/



	protected:

		int _health;
		int _shield;

		bool _weapons[];

	}; // class CHudOverlay

	REG_FACTORY(CHudOverlay);

} // namespace Logic

#endif // __Logic_Life_H
