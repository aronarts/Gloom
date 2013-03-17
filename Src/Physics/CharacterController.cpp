//---------------------------------------------------------------------------
// CharacterController.cpp
//---------------------------------------------------------------------------

/**
@file CharacterController.cpp

@see Graphics::CCharacterController

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#include "CharacterController.h"

#include <assert.h>

#include "Physics/Server.h"
#include "Physics/CollisionManager.h"
#include "Conversions.h"

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxShape.h>
#include <PxRigidDynamic.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <characterkinematic/PxCapsuleController.h>
#include <characterkinematic/PxController.h>
#include <characterkinematic/PxControllerManager.h>

using namespace physx;

namespace Physics {

	CCharacterController::CCharacterController() {
		// Obtenemos la sdk de physics y comprobamos que ha sido inicializada
		_physxSDK = Physics::CServer::getSingletonPtr()->getPhysxSDK();
		assert(_physxSDK && "No se ha inicializado physX");
		
		// Obtenemos la escena de physx y comprobamos que ha sido inicializada
		_scene = Physics::CServer::getSingletonPtr()->getActiveScene();
		assert(_scene && "No existe una escena fisica");

		_controllerManager = Physics::CServer::getSingletonPtr()->getControllerManager();
		assert(_controllerManager && "No existe controller manager");

		_collisionManager = Physics::CServer::getSingletonPtr()->getCollisionManager();
	} // CCharacterController

	//________________________________________________________________________

	CCharacterController::~CCharacterController() {
		// Destruimos el actor de physx asociado al controller y desligamos el 
		// actor de la escena
		if(_controller != NULL) {
			_controller->release();
			_controller = NULL;
		}

		// Fijamos los punteros a physx como nulos
		_physxSDK = NULL;
		_scene = NULL;
		_controllerManager = NULL;
		_collisionManager = NULL;
	} // ~CCharacterController

	//________________________________________________________________________

	void CCharacterController::load(const Vector3 &position, float radius, float height, 
									int group, const std::vector<int>& groupList, 
									const Logic::CPhysicController* component) {

		assert(_scene);

		// Nota: PhysX coloca el sistema de coordenadas local en el centro de la c�psula, mientras
		// que la l�gica asume que el origen del sistema de coordenadas est� en los pi�s del 
		// jugador. Para unificar necesitamos realizar una traslaci�n en el eje Y.
		// Desafortunadamente, el descriptor que se usa para crear los controllers no permite
		// definir esta transformaci�n local (que s� permite al crear un actor), por lo que
		// tendremos que realizar la traslaci�n nosotros cada vez. 

		// Transformaci�n entre el sistema de coordenadas l�gico y el de PhysX
		float offsetY = height / 2.0f + radius;
		PxVec3 pos = Vector3ToPxVec3(position + Vector3(0, offsetY, 0));
	
		// Crear descriptor del controller
		PxCapsuleControllerDesc desc;
		desc.position = PxExtendedVec3(pos.x, pos.y, pos.z);
		desc.height = height;
		desc.radius = radius;
		desc.material = _physxSDK->createMaterial(0.5f, 0.5f, 0.1f); // En realidad sera getDefaultMaterial en el futuro
		desc.climbingMode = PxCapsuleClimbingMode::eEASY;

		//desc.contactOffset <-- Tama�o que recubre el volumen de la capsula, physx lo usa para no permitir que la capsula
		//desc.stepOffset <-- Tama�o de los escalones que el player puede subir
		// se introduzca en otros shapes
		//desc.slopeLimit = 0.707f;

		desc.callback = _collisionManager;   // Establecer gestor de colisiones
		desc.userData = (void *) component;  // Anotar el componente l�gico asociado al controller

		// Retorna un PxController que podemos castear a capsule controller por ser nuestro caso
		_controller = static_cast<PxCapsuleController*>( _controllerManager->createController(*_physxSDK, _scene, desc) );
	
		// Anotar el componente l�gico asociado al actor dentro del controller (No es autom�tico)
		_controller->getActor()->userData = (void *) component;

		// Establecer el grupo de colisi�n
		PxSetGroup(*_controller->getActor(), group);
		Physics::CServer::getSingletonPtr()->setupFiltering(_controller->getActor(), group, groupList);
	}

	//________________________________________________________________________

	unsigned CCharacterController::move(const Vector3 &movement, unsigned int msecs) {
		// Vector de desplazamiento, nosotros somos los encargados de sumarle la fuerza
		// de la gravedad, ya que physX no lo hace por nosotros.
		PxVec3 disp = Vector3ToPxVec3(movement);

		// Movemos el character controller y retornamos un entero con los flags de colision (PxControllerFlag)
		// Fijamos la distancia minima a la que parar el algoritmo de movimiento a 0.01f.
		// Pasamos el tiempo de frame transcurrido en micro segundos (como a physX le gusta).
		// Dado que no tenemos objetos fisicos no manejados por physX, pasamos NULL como obstaculo.
		return _controller->move(disp, 0.01f, msecs * 0.001f, _filters, NULL);
	}

	//________________________________________________________________________

	Vector3 CCharacterController::getPosition() {
		// Antes de devolver la posici�n del controller debemos transformar entre el 
		// sistema de coordenadas de PhysX y el de la l�gica
		float offsetY = ( _controller->getHeight() * 0.5f ) + _controller->getRadius();
		Vector3 pos = PxExtendedVec3ToVector3( _controller->getPosition() );
		
		return pos - Vector3(0, offsetY, 0);
	}

	//________________________________________________________________________

	void CCharacterController::setPosition(const Vector3& position) {
		// Transformaci�n entre el sistema de coordenadas l�gico y el de PhysX
		float offsetY = ( _controller->getHeight() * 0.5f ) + _controller->getRadius();
		PxVec3 pos = Vector3ToPxVec3(position + Vector3(0, offsetY, 0));

		// Transformamos el vector a uno de mayor precision que physx utiliza por debajo
		// para manejar a los controllers con mayor exactitud en mundos muy grandes
		_controller->setPosition( PxExtendedVec3(pos.x, pos.y, pos.z) );
	}

	//________________________________________________________________________

	void CCharacterController::activateSimulation() {
		// Activamos todos los shapes del componente por completo en PhysX
		// Para ello, obtenemos todos sus shapes y ponemos los flags a true

		int nbShapes = _controller->getActor()->getNbShapes();
		PxShape** actorShapes = new PxShape* [nbShapes];
		_controller->getActor()->getShapes(actorShapes, nbShapes);
		for(int i = 0; i < nbShapes; ++i) {
			// Esta shape tomara parte en barridos, raycasts...
			actorShapes[i]->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			// Esta shape entrara dentro de la simulacion de fisicas
			actorShapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE , true);
		}
		_controller->setInteraction(PxCCTInteractionMode::eINCLUDE);
		delete [] actorShapes;
	}

	//________________________________________________________________________

	void CCharacterController::deactivateSimulation() {
		// Desactivamos todos los shapes del componente por completo en PhysX
		// Para ello, obtenemos todos sus shapes y ponemos los flags a false

		int nbShapes = _controller->getActor()->getNbShapes();
		PxShape** actorShapes = new PxShape* [nbShapes];
		_controller->getActor()->getShapes(actorShapes, nbShapes);
		for(int i = 0; i < nbShapes; ++i) {
			// Esta shape no tomara parte en barridos, raycasts...
			actorShapes[i]->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			// Esta shape no entrara dentro de la simulacion de fisicas
			actorShapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE , false);
		}
		_controller->setInteraction(PxCCTInteractionMode::eEXCLUDE);
		delete [] actorShapes;
	}

} // namespace Physics
