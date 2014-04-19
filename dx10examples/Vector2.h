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

	inline float Dot(Vector2 v)
	{
		float value = X * v.X + Y * v.Y;
		return value;
	}

	inline Vector2 operator - ( Vector2  v )
	{
		float newX = X - v.X;
		float newY = Y - v.Y;

		return Vector2(newX, newY);
	}

	inline Vector2 operator + ( Vector2  v )
	{
		float newX = X + v.X;
		float newY = Y + v.Y;

		return Vector2(newX, newY);
	}
};

#endif
