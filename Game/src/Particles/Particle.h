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
	float StartTime;
	bool IsDead;
	float Gravity;
	bool FlippedHorizontal;
	bool FlippedVertical;
	float Brightness; // value between 0 and 1.0

	float CurrentAlpha; // needed to stop the alpha from flickering

	Particle(void)
	{
		 PosX = 0;
		 PosY = 0;
		 StartPosX = 0;
		 StartPosY = 0;
		 DirectionX = 0;
		 DirectionY = 0;
		 Size = 0;
		 StartSize = 0;
		 Speed = 0;
		 StartSpeed = 0;
		 MaxLiveTime = 0;
		 StartTime = 0;
		 IsDead = false;
		 Gravity = 0;
		 FlippedHorizontal = false;
		 FlippedVertical = false;
		 Brightness = 1.0;
	}

	~Particle(void)
	{
	}
};

#endif
