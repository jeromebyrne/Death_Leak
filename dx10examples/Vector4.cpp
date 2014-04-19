#include "precompiled.h"
#include "Vector4.h"

Vector4::Vector4(void): Vector3(0,0,0), W(0)
{
}

Vector4::Vector4(float x, float y, float z, float w):Vector3(x,y,z), W(w)
{
}

Vector4::~Vector4(void)
{
}
