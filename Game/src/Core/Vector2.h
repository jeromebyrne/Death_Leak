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

	inline float Cross(Vector2 v)
	{
		return (X * v.Y) - (Y * v.X);
	}

	inline static Vector2 Cross(Vector2 v1, Vector2 v2)
	{
		return (v1.X * v2.Y) - (v1.Y * v2.X);
	}

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

	inline void operator += (Vector2 & value)
	{
		X += value.X;
		Y += value.Y;
	}

	inline void operator -= (Vector2 & value)
	{
		X -= value.X;
		Y -= value.Y;
	}

	inline Vector2 operator * (float value)
	{
		float newX = X * value;
		float newY = Y * value;

		return Vector2(newX, newY);
	}

	inline Vector2 operator * (Vector2 & value)
	{
		Vector2 returnValue(X * value.X, Y * value.Y);

		return returnValue;
	}

	inline Vector2 operator / (float value)
	{
		float newX = X / value;
		float newY = Y / value;

		return Vector2(newX, newY);
	}

	float Distance(Vector2 dest);

	static float Distance(Vector2 src, Vector2 dest);

	float LengthSquared();
};

#endif
