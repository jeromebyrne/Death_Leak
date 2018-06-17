#ifndef PARTICLEEMITTERMANAGER_H
#define PARTICLEEMITTERMANAGER_H
#include "Graphics.h"

class ParticleEmitterManager
{
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

	ParticleSpray * CreateRadialBloodSpray(unsigned int numParticles,
											Vector2 position,
											bool loop,
											float loopTime);

	ParticleSpray * CreateDirectedBloodSpray(int numParticles,
								 Vector2 position, 
								 Vector2 direction,
								 float spread,
								 bool loop,
								 float loopTime);

	//void Update(); // update all of our particle sprays
	ParticleSpray * CreateRadialSpray(int numParticles,
						 Vector2 position, 
						 GameObject::DepthLayer depthLayer,
						 Vector2 drawBoundingBox, // determines when we stop drawing particles (camera stops seeing them)
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
						 bool scalesByLiveTime,
						 float scaleTo,
						 float fadeInPercentTime,
						 float fadeOutPercentTime,
						 float spawnOffsetX,
						 float spawnOffsetY); 


	ParticleSpray * CreateDirectedSpray(int numParticles,
										 Vector2 position,
										 GameObject::DepthLayer depthLayer,
										 Vector2 direction,
										 float spread,
										 Vector2 drawBoundingBox, // determines when we stop drawing particles (camera stops seeing them)
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
										 bool scalesByLiveTime,
										 float scaleTo,
										 float spawnSpreadX,
										 float spawnSpreadY,
										 float fadeInPercentTime,
										 float fadeOutPercentTime,
										 bool originalOrientation = false);

	

	ParticleSpray * CreateDirectedSprayLoadTime(int numParticles,
												 Vector2 position, 
												 GameObject::DepthLayer depthLayer,
												 Vector2 direction,
												 float spread,
												 Vector2 drawBoundingBox, // determines when we stop drawing particles (camera stops seeing them)
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
												 bool scalesByLiveTime,
												 float scaleTo,
												 float spawnSpreadX,
												 float spawnSpreadY,
												 float fadeInPercentTime,
												 float fadeOutPercentTime);

	private:

		static ParticleEmitterManager * m_instance;

		ParticleEmitterManager(void);
		~ParticleEmitterManager(void);

		list<ParticleSpray*> m_particleSprayList; // a list of all active particle sprays
		Graphics * m_graphicsSystem; // a pointer to the graphics system	
};

#endif
