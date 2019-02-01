#ifndef PARTICLE_H
#define PARTICLE_H

// this is basically a structure that holds individual particle info
class Particle
{
public:
	float PosX;
	float PosY;
	float StartPosX;
	float StartPosY;
	float DirectionX;
	float DirectionY;
	float Size;
	float StartSize;
	float Speed;
	float StartSpeed;
	float MaxLiveTime;
	float CurrentTime;
	bool IsDead;
	float Gravity;
	float Brightness; // value between 0 and 1.0
	float PosXOffset = 0.0f;
	float PosYOffset = 0.0f;

	float CurrentAlpha; // needed to stop the alpha from flickering

	Particle(void) :
		 PosX(0.0f),
		 PosY(0.0f),
		 StartPosX(0.0f),
		 StartPosY(0.0f),
		 DirectionX(0.0f),
		 DirectionY(0.0f),
		 Size(0.0f),
		 StartSize(0.0f),
		 Speed(0.0f),
		 StartSpeed(0.0f),
		 MaxLiveTime(0.0f),
		 CurrentTime(0.0f),
		 IsDead(false),
		 Gravity(0.0f),
		 Brightness(1.0f)
	{
	}

	~Particle(void)
	{
	}
};

#endif
