#ifndef SMASHABLE_H
#define SMASHABLE_H

#include "solidmovingsprite.h"

class Smashable : public SolidMovingSprite
{
public:

	enum SmashableState
	{
		kNotSmashed,
		kSmashed
	};

	Smashable(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~Smashable(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;

	SmashableState GetSmashableState() const { return mState; }
	
private:

	void UpdateTextureForState();

	virtual void UpdateAnimations() override;

	void SpawnSmashedParticles();

	SmashableState mState;
};

#endif
