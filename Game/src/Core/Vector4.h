#ifndef VECTOR4_H
#define VECTOR4_H
#include "vector3.h"

class Vector4 : public Vector3
{
public:
	float W;

	Vector4(void);
	Vector4(float x = 0, float y = 0, float z = 0, float w = 0);
	~Vector4(void);
};
#endif
