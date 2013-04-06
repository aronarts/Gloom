//---------------------------------------------------------------------------
// Shadow.h
//---------------------------------------------------------------------------

/**
@file Shadow.h

Contiene la declaraci�n de la clase que
implementa las habilidades del personaje
"Shadow".

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#ifndef __Logic_Shadow_H
#define __Logic_Shadow_H

#include "BaseSubsystems/Math.h"
#include "PlayerClass.h"

namespace Logic {

	/**
	Clase que implementa las habilidades propias
	del personaje "Shadow".

	@ingroup logicGroup

	@author Francisco Aisa Garc�a
	@date Febrero, 2013
	*/

	class CShadow : public CPlayerClass {
		DEC_FACTORY(CShadow);
	public:


		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================


		/** Constructor por defecto. */
		CShadow();

		//__________________________________________________________________

		/** Destructor virtual. */
		virtual ~CShadow();
		

		// =======================================================================
		//                    METODOS HEREDADOS DE ICOMPONENT
		// =======================================================================
	

		/**
		Inicializaci�n del componente a partir de la informaci�n extraida de la entidad
		leida del mapa:
		<ul>
			<li><strong>shadowInvisibilityDuration:</strong> Tiempo que dura la invsibilidad desde que se activa. </li>
		</ul>

		@param entity Entidad a la que pertenece el componente.
		@param map Mapa L�gico en el que se registrar� el objeto.
		@param entityInfo Informaci�n de construcci�n del objeto le�do del fichero de disco.
		@return Cierto si la inicializaci�n ha sido satisfactoria.
		*/
		virtual bool spawn(CEntity* entity, CMap* map, const Map::CEntity *entityInfo);

		//__________________________________________________________________

		/**
		M�todo llamado en cada frame.

		@param msecs Milisegundos transcurridos desde el �ltimo tick.
		*/
		virtual void tick(unsigned int msecs);

		//__________________________________________________________________

		/**
		Metodo que se llama al activar el componente.
		Restea los campos de la clase a los valores por defecto.
		*/
		virtual void activate();


		// =======================================================================
		//                  METODOS HEREDADOS DE CPlayerClass
		// =======================================================================


		/** El personaje se vuelve invisible al invocar este m�todo. */
		virtual void primarySkill();

		//__________________________________________________________________

		/** Habilidad por definir. */
		virtual void secondarySkill();


	private:


		// =======================================================================
		//                          MIEMBROS PRIVADOS
		// =======================================================================


		/** Tiempo que dura la invisibilidad. */
		float _invisibilityDuration;

		/** Timer que controla la duraci�n de la invisibilidad. */
		float _invisibilityTimer;

	}; // class CShadow

	REG_FACTORY(CShadow);

} // namespace Logic

#endif // __Logic_Shadow_H