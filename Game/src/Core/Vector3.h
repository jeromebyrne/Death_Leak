#ifndef VECTOR3_H
#define VECTOR3_H

class Vector3
{
private:

	public:
	
	float X;
	float Y;
	float Z;

	Vector3();
	Vector3(float x, float y, float z);
	~Vector3(void);


	inline Vector3 operator - ( Vector3  & v )
	{
		float newX = X - v.X;
		float newY = Y - v.Y;
		float newZ = Z - v.Z;

		return Vector3(newX, newY, newZ);
	}
	inline Vector3 operator + ( Vector3  & v )
	{
		float newX = X + v.X;
		float newY = Y + v.Y;
		float newZ = Z + v.Z;

		return Vector3(newX, newY, newZ);
	}

	inline Vector3 operator * (float value)
	{
		float newX = X * value;
		float newY = Y * value;
		float newZ = Z * value;

		return Vector3 (newX, newY, newZ);
	}

	inline Vector3 operator * (Vector3 & value)
	{
		Vector3 returnValue(X * value.X, Y * value.Y, Z * value.Z);

		return returnValue;
	}

	inline Vector3 operator / (float value)
	{
		float newX = X / value;
		float newY = Y / value;
		float newZ = Z / value;

		return Vector3 (newX, newY, newZ);
	}

	inline void operator += (Vector3 & value)
	{
		X += value.X;
		Y += value.Y;
		Z += value.Z;
	}

	inline void operator -= (Vector3 & value)
	{
		X -= value.X;
		Y -= value.Y;
		Z -= value.Z;
	}

	inline float Dot(Vector3 & v)
	{
		float value = X * v.X + Y * v.Y + Z * v.Z;
		return value;
	}
	
	inline Vector3 Cross(Vector3 & v)
	{
		return Vector3(Y * v.Z - Z * v.Y, Z * v.X - X * v.Z, X * v.Y - Y * v.X);
	}

	inline static Vector3 Cross(Vector3 & v1, Vector3 & v2)
	{
		return Vector3(v1.Y * v2.Z - v1.Z * v2.Y, v1.Z * v2.X - v1.X * v2.Z, v1.X * v2.Y - v1.Y * v2.X);
	}

	// returns the length of the vector
	float Length();

	float LengthSquared();

	// make unit length
	void Normalise();
	float Distance(Vector3 destination);
	static float Distance(Vector3 source, Vector3 destination);


};

#endif
