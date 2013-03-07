/**
@file Light.h

Contiene la declaraci�n del componente que controla la vida de una entidad.

@see Logic::CLight
@see Logic::IComponent

@author Antonio Jesus Narvaez
@date Marzo, 2013
*/
#ifndef __Logic_Light_H
#define __Logic_Light_H

#include "Graphics/Light.h"
#include "Logic/Entity/Component.h"

//declaraci�n de la clase
namespace Logic 
{
/**
	Este componente controla la luz que tiene un objeto. 
	
    @ingroup logicGroup

	@author Antonio Jesus Narvaez
	@date Marzo, 2013
*/
	class CLight : public IComponent
	{
		DEC_FACTORY(CLight);
	public:

		/**
		Constructor por defecto; en la clase base no hace nada.
		*/
		CLight() : IComponent() {}

		/**
		M�todo llamado en cada frame que actualiza el estado del componente de luz.
				

		@param msecs Milisegundos transcurridos desde el �ltimo tick.
		*/
		//virtual void tick(unsigned int msecs);
		/**
		Inicializaci�n del componente usando la descripci�n de la entidad que hay en 
		el fichero de mapa.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		/**
		Metodo que se llama al activar el componente.
		*/
		virtual void activate();

		/**
		mensajes aceptados por el componente
		*/
		virtual bool accept(CMessage *message);

		/**
		
		*/
		virtual void process(CMessage *message);

		

	protected:

		/**
		Puntero al objeto luz
		*/
		Graphics::CLight *_light;

	}; // class CLight

	REG_FACTORY(CLight);

} // namespace Logic

#endif // __Logic_Light_H