#include "precompiled.h"
#include "Vector3.h"

Vector3::Vector3() : X(0), Y(0), Z(0)
{
}
Vector3::Vector3(float x = 0, float y = 0, float z = 0): X(x), Y(y), Z(z)
{
}

Vector3::~Vector3(void)
{
}
float Vector3::Distance(Vector3 dest)
{
	float X2_X1_plus_Y2_Y1;

	X2_X1_plus_Y2_Y1 = (dest.X - X) * (dest.X - X) + (dest.Y - Y) * (dest.Y - Y) + (dest.Z - Z) * (dest.Z - Z);

	float distance = sqrt(X2_X1_plus_Y2_Y1);

	return distance;
}

float Vector3::Distance(Vector3 src, Vector3 dest)
{
	float X2_X1_plus_Y2_Y1;

	X2_X1_plus_Y2_Y1 = (dest.X - src.X) * (dest.X - src.X) + (dest.Y - src.Y) * (dest.Y - src.Y) + (dest.Z - src.Z) * (dest.Z - src.Z);

	float distance = sqrt(X2_X1_plus_Y2_Y1);

	return distance;
}
float Vector3::Length()
{
	float value = sqrt((X * X) + (Y * Y) + (Z * Z));
	return value;
}
void Vector3::Normalise()
{
	float length = Length();
	X /= length;
	Y /= length;
	Z /= length;
}
float Vector3::LengthSquared()
{
	float value = X*X + Y*Y + Z*Z;
	return value;
}
