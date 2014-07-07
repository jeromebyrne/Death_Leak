#ifndef VECTOR2_H
#define VECTOR2_H

class Vector2
{
public:
	float X;
	float Y;
	Vector2(float x = 0, float y = 0);
	~Vector2(void);

	float Length();
	void Normalise();

	inline float Dot(Vector2 & v)
	{
		float value = X * v.X + Y * v.Y;
		return value;
	}

	/*
	inline Vector2 Cross(Vector3 v)
	{
		return Vector2(Y * v.Z - Z * v.Y, Z * v.X - X * v.Z, X * v.Y - Y * v.X);
	}

	inline static Vector2 Cross(Vector3 v1, Vector3 v2)
	{
		return Vector2(v1.Y * v2.Z - v1.Z * v2.Y, v1.Z * v2.X - v1.X * v2.Z, v1.X * v2.Y - v1.Y * v2.X);
	}
	*/

	inline Vector2 operator - ( Vector2 & v )
	{
		float newX = X - v.X;
		float newY = Y - v.Y;

		return Vector2(newX, newY);
	}

	inline Vector2 operator + ( Vector2  & v )
	{
		float newX = X + v.X;
		float newY = Y + v.Y;

		return Vector2(newX, newY);
	}

	float LengthSquared();
};

#endif
