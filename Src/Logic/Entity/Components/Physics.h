/**
@file Physics.h

Contiene la declaraci�n de una clase abstracta de la que deben heredar todos los
componentes f�sicos.

@see Logic::IComponent
@see Logic::CPhysicEntity
@see Logic::CPhysicController

@author Antonio S�nchez Ruiz-Granados
@date Noviembre, 2012
*/

#ifndef __Logic_Physics_H
#define __Logic_Physics_H
//
#include "Logic/Entity/Component.h"
#include <iostream>

// Predeclaraci�n de tipos
namespace physx {
	struct PxTriggerPair;
};

// Los componentes pertenecen al namespace Logic
namespace Logic 
{	
	/**
	Clase abstracta de la que deben heredar todos los componentes f�sicos. Proporciona un interfaz
	com�n para recibir eventos desde el motor de f�sica.
	
    @ingroup logicGroup

	@author Antonio S�nchez Ruiz-Granados
	@date Noviembre, 2012
	*/
	class IPhysics : public IComponent
	{
	public:
 
		/**
		Este m�todo es invocado desde el motor de f�sica cuando una entidad entra o sale de un
		trigger f�sico. Se notifica tanto al componente asociado al trigger como al componente
		asociado a la otra entidad.

		@param otherComponent Componente asociado al trigger o a la otra entidad, dependiendo
		       de a qui�n se est� notificando.
		@param enter True si la entidad entra en el trigger y false si sale. 
		*/
		virtual void onTrigger(IPhysics *otherComponent, bool enter) = 0;

		/**
		Este m�todo es invocado desde el motor de f�sica cuando una entidad golpea a otra. Para
		comprobar exclusivamente si un controller a golpeado a otra cosa es mejor usar otros
		metodos de physX.

		Para mejorar la eficiencia de esta query, physX permite implementar filtros, de manera
		que cada actor fisico pueda indicar de que cosas quiere ser notificado (pudiendo eliminar
		onContact por ejemplo, en algunas entidades).

		@param otherComponent Componente asociado a la otra entidad que nos ha golpeado.
		*/
		virtual void onContact(IPhysics *otherComponent) = 0;

	}; // class IPhysics

} // namespace Logic

#endif // __Logic_Physics_H
