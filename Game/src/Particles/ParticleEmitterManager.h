#ifndef PARTICLEEMITTERMANAGER_H
#define PARTICLEEMITTERMANAGER_H
#include "Graphics.h"

class ParticleEmitterManager
{
private:
	static ParticleEmitterManager * m_instance;

	ParticleEmitterManager(void);
	~ParticleEmitterManager(void);

	list<ParticleSpray*> m_particleSprayList; // a list of all active particle sprays
	Graphics * m_graphicsSystem; // a pointer to the graphics system

	static unsigned int sNumParticlesInWorld;

public:

	inline static ParticleEmitterManager * Instance()
	{
		if(m_instance == 0)
		{
			m_instance = new ParticleEmitterManager(); 
		}
		return m_instance;
	}
	void ClearParticles();

	void Initialise(Graphics * graphicsSystem); 

	static void DecrementParticleWorldCount(unsigned int numParticles);

	ParticleSpray * CreateRadialBloodSpray(unsigned int numParticles,
											Vector3 position,
											bool loop,
											float loopTime);

	ParticleSpray * CreateDirectedBloodSpray(int numParticles,
								 Vector3 position, 
								 Vector3 direction,
								 float spread,
								 bool loop,
								 float loopTime);

	//void Update(); // update all of our particle sprays
	void CreateRadialSpray(int numParticles,
						 Vector3 position, 
						 Vector3 drawBoundingBox, // determines when we stop drawing particles (camera stops seeing them)
						 string textureFileName,
						 float minSpeed,
						 float maxSpeed,
						 float minLiveTime,
						 float maxLiveTime,
						 float minSize,
						 float maxSize,
						 float gravity,
						 bool loop,
						 float minBrightness,
						 float maxBrightness,
						 float loopTime,
						 bool scalesByLiveTime = false,
						 float scaleTo = 1); 


	ParticleSpray * CreateDirectedSpray(int numParticles,
										 Vector3 position, 
										 Vector3 direction,
										 float spread,
										 Vector3 drawBoundingBox, // determines when we stop drawing particles (camera stops seeing them)
										 string textureFileName,
										 float minSpeed,
										 float maxSpeed,
										 float minLiveTime,
										 float maxLiveTime,
										 float minSize,
										 float maxSize,
										 float gravity,
										 bool loop,
										 float minBrightness,
										 float maxBrightness,
										 float loopTime,
										 bool scalesByLiveTime = false,
										 float scaleTo = 1); 

	

	ParticleSpray * CreateDirectedSprayLoadTime(int numParticles,
												 Vector3 position, 
												 Vector3 direction,
												 float spread,
												 Vector3 drawBoundingBox, // determines when we stop drawing particles (camera stops seeing them)
												 string textureFileName,
												 float minSpeed,
												 float maxSpeed,
												 float minLiveTime,
												 float maxLiveTime,
												 float minSize,
												 float maxSize,
												 float gravity,
												 bool loop,
												 float minBrightness,
												 float maxBrightness,
												 float loopTime,
												 bool scalesByLiveTime = false,
												 float scaleTo = 1);
	
};

#endif
