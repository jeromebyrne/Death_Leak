#ifndef PARTICLESPRAY_H
#define PARTICLESPRAY_H

#include "drawableobject.h"
#include "particle.h"
#include "effectbloodparticlespray.h"


class ParticleSpray : public DrawableObject
{
protected:
	ID3D10ShaderResourceView* m_texture; // the texture being displayed on each quad of this spray
	std::string m_textureFilename; // the filename of the texture

	ID3D10Buffer* m_vertexBuffer; // buffer to hold all of the particle vertices

	void SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[]); // sets the vertex buffer
	EffectParticleSpray * m_effectParticleSpray; // the default particle spray effect
	EffectBloodParticleSpray * m_effectBloodParticleSpray; // the specialised blood shader
	EffectParticleSpray * m_currentEffect;

	list<Particle> m_particleList; // a list of particle structures
	int m_numAliveParticles; // the number of particles still alive

	bool m_isLooping; // do we do a continous spray of particles?
	float m_loopTime; // how long should we loop for? if 0 then we loop forever
	float m_startedLooping; // at what time did we start looping

	bool m_scalesByLiveTime; // should we scale up the longer we live?
	float m_scaleTo; // how big should we scale to based on our live time

	// if we're attached to a sprite
	Sprite * m_parent;
	Vector2 m_parentOffset;
	bool m_parentHFlipInitial; // was the parent flipped on the initial attach

	bool mIsBloodSpray;

	Vector2 m_direction;
	float m_spread;
	float m_minSpeed;
	float m_maxSpeed;
	float m_minLivetime;
	float m_maxLivetime;
	float m_minSize;
	float m_maxSize;
	float m_gravity;
	float m_minBrightness;
	float m_maxBrightness;
	float m_numParticles;
public:
	ParticleSpray(bool isBloodSpray, 
					Vector3 position,
					Vector3 dimensions, 
					const char* textureFileName, 
					list<Particle> particles,
					bool isLooping,
					unsigned long loopTime,
					bool scaleByLiveTime, 
					float scaleTo);

	ParticleSpray();
	virtual ~ParticleSpray(void);

	virtual void Draw(ID3D10Device* device, Camera2D * camera) override;
	virtual void Update(float delta) override;
	virtual void LoadContent(ID3D10Device* device) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	void AttachToSprite(Sprite * parent, Vector2 offset);
	void DetachFromSprite();

	virtual void DebugDraw(ID3D10Device *  device) override { /*Don't draw debug info for particles as it's too confusing*/ }

	void SetGeneralDirectionValue(Vector2 value) { m_direction = value; }
	void SetSpreadValue(float value) { m_spread = value; }
	void SetMinSpeedValue(float value) { m_minSpeed = value; }
	void SetMaxSpeedValue(float value) { m_maxSpeed = value; }
	void SetMinLiveTime(float value) { m_minLivetime = value; }
	void SetMaxLiveTime(float value) { m_maxLivetime = value; }
	void SetMinSize(float value) { m_minSize = value; }
	void SetMaxSize(float value) { m_maxSize = value; }
	void SetGravityValue(float value) { m_gravity = value; }
	void SetMinBrightnessValue(float value) { m_minBrightness = value; }
	void SetMaxBrightnessValue(float value) { m_maxBrightness = value; }
	void SetNumParticlesValue(float value) { m_numParticles = value; }
};

#endif