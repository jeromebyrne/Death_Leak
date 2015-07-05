#ifndef AMBIENTBIRD_H
#define AMBIENTBIRD_H

#include "npc.h"

class AmbientBird : public NPC
{
public:
	AmbientBird(void);
	virtual ~AmbientBird(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	virtual void UpdateAnimations() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void Update(float delta) override;

	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	bool mIsGliding;

	float mTimeUntilFlap;

	float mTimeUntilEndFlap;

	float mTravelOffset;

	Vector3 mStartPosition;
};

#endif