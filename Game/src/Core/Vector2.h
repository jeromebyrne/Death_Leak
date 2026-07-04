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

	inline float Dot(const Vector2 & v) const
	{
		float value = X * v.X + Y * v.Y;
		return value;
	}

	inline float Cross(const Vector2 & v)
	{
		return (X * v.Y) - (Y * v.X);
	}

	inline static Vector2 Cross(const Vector2 & v1, const Vector2 & v2)
	{
		return (v1.X * v2.Y) - (v1.Y * v2.X);
	}

	inline Vector2 operator - ( const Vector2 & v ) const
	{
		float newX = X - v.X;
		float newY = Y - v.Y;

		return Vector2(newX, newY);
	}

	inline Vector2 operator + ( const Vector2  & v ) const
	{
		float newX = X + v.X;
		float newY = Y + v.Y;

		return Vector2(newX, newY);
	}

	inline void operator += (const Vector2 & value)
	{
		X += value.X;
		Y += value.Y;
	}

	inline void operator -= (const Vector2 & value)
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

	inline Vector2 operator * (const Vector2 & value) const
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
