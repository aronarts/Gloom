/**
@file GeometryFactory.cpp

@see Logic::CGeometryFactory

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#include "GeometryFactory.h"

#include "Conversions.h"

#include <cassert>

using namespace physx;

namespace Physics {

	CGeometryFactory* CGeometryFactory::_instance = 0;
	
	//________________________________________________________________________

	CGeometryFactory::CGeometryFactory() {
		_instance = this;
	} // CGeometryFactory
	
	//________________________________________________________________________

	CGeometryFactory::~CGeometryFactory() {
		_instance = 0;
	} // ~CGeometryFactory
	
	//________________________________________________________________________

	bool CGeometryFactory::Init() {
		assert(!_instance && "Segunda inicializaci�n de Physics::CGeometryFactory no permitida!");

		return new CGeometryFactory();
	} // Init
	
	//________________________________________________________________________

	void CGeometryFactory::Release() {
		assert(_instance && "Physics::CGeometryFactory no est� inicializado!");

		if(_instance) {
			delete _instance;
		}
	} // Release

	//________________________________________________________________________

	PxSphereGeometry CGeometryFactory::createSphere(float radius) {
		return PxSphereGeometry(radius);
	}

	//________________________________________________________________________

	PxBoxGeometry CGeometryFactory::createBox(const Vector3& dimensions) {
		return PxBoxGeometry( Vector3ToPxVec3(dimensions) );
	}

	//________________________________________________________________________

	PxPlane CGeometryFactory::createPlane(const Vector3 &point, const Vector3 &normal) {
		return PxPlane( Vector3ToPxVec3(point), Vector3ToPxVec3(normal) );
	}

} // namespace Physics
