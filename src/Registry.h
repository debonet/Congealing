#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include "Point2D.h"
#include "Point3D.h"
#include "Vector.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

template <class STORAGE>
class RegistryOf
	: public VectorOf<STORAGE>
{
public:
	void Register(const STORAGE& storage)
	{
		VectorOf<STORAGE>::Append(storage);
	}

	void Register(const PointOf<2,STORAGE> &pt){
		Register(pt.X());
		Register(pt.Y());
	}
	void Register(const PointOf<3,STORAGE> &pt){
		Register(pt.X());
		Register(pt.Y());
		Register(pt.Z());
	}

};

#endif
