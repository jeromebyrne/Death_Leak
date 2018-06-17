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

float Vector2::LengthSquared()
{
	float value = X*X + Y*Y;
	return value;
}

float Vector2::Distance(Vector2 dest)
{
	float X2_X1_plus_Y2_Y1;

	X2_X1_plus_Y2_Y1 = (dest.X - X) * (dest.X - X) + (dest.Y - Y) * (dest.Y - Y);

	float distance = sqrt(X2_X1_plus_Y2_Y1);

	return distance;
}

float Vector2::Distance(Vector2 src, Vector2 dest)
{
	float X2_X1_plus_Y2_Y1;

	X2_X1_plus_Y2_Y1 = (dest.X - src.X) * (dest.X - src.X) + (dest.Y - src.Y) * (dest.Y - src.Y);

	float distance = sqrt(X2_X1_plus_Y2_Y1);

	return distance;
}
