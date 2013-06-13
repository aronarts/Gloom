//---------------------------------------------------------------------------
// CoolDown.h
//---------------------------------------------------------------------------

/**
@file CoolDown.h

Contiene la declaraci�n del componente que implementa el coolDown.

@author Jose Antonio Garc�a Y��ez
@date Febrero, 2013
*/

#ifndef __Logic_CoolDown_H
#define __Logic_CoolDown_H

#include "Logic/Entity/Components/Spell.h"

namespace Logic {
	
	/**
	@ingroup logicGroup

	Este componente implementa la funcionalidad del coolDown. Tan solo necesitamos
	redefinir los mensajes que se mandan a las entidades en caso de hit, ya que 
	el resto del comportamiento esta definido en el arquetipo que describe 
	al coolDown.

	@author Jose Antonio Garc�a Y��ez
	@date Febrero, 2013
	*/

	class CCoolDown : public ISpell {
		DEC_FACTORY(CCoolDown);

	public:

		/** Constructor por defecto. */
		CCoolDown() : ISpell("coolDown"), _percentage(0) { }

		//__________________________________________________________________

		virtual ~CCoolDown();

		//__________________________________________________________________

		/**
		Inicializaci�n del componente utilizando la informaci�n extra�da de
		la entidad le�da del mapa (Maps::CEntity). Toma del mapa el atributo
		speed que indica a la velocidad a la que se mover� el jugador.

		Inicializaci�n del componente a partir de la informaci�n extraida de la entidad
		leida del mapa:
		<ul>
			<li><strong>spellCoolDownDamageReflect:</strong> Da�o que hace el coolDown al golpear contra el mundo. </li>
		</ul>

		@param entity Entidad a la que pertenece el componente.
		@param map Mapa L�gico en el que se registrar� el objeto.
		@param entityInfo Informaci�n de construcci�n del objeto le�do del fichero de disco.
		@return Cierto si la inicializaci�n ha sido satisfactoria.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		/**
		M�todo virtual puro que debe ser implementado por las clases derivadas para
		especificar que ocurre al usar el disparo primario.
		*/
		virtual void spell();

		//__________________________________________________________________
		
		/**
		Este m�todo es invocado cuando se deja de pulsar el bot�n de disparo
		primario.
		*/
		virtual void stopSpell();

	private:
		
		// porcentaje que recudira el cooldown
		float _percentage;

	}; // class CShootRaycast

	REG_FACTORY(CCoolDown);

} // namespace Logic

#endif // __Logic__H
