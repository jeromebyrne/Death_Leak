#include "precompiled.h"
#include "Vector2.h"

Vector2::Vector2(float x, float y): X(x), Y(y)
{
}

Vector2::~Vector2(void)
{
}

float Vector2::Length()
{
	float value = sqrt((X * X) + (Y * Y));
	return value;
}
void Vector2::Normalise()
{
	float length = Length();
	X /= length;
	Y /= length;
}
