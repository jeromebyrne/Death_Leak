#ifndef BREAKABLE_H
#define BREAKABLE_H

#include "solidmovingsprite.h"

class Breakable : public SolidMovingSprite
{
public:

	enum BreakableState
	{
		kNoDamage,
		kSlightDamage,
		kDamaged,
		kHeavyDamaged,
		kBroken
	};

	Breakable(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10);
	virtual ~Breakable(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) override;

	float GetHealth() const { return mHealth; }
	float GetMaxHealth() const { return mMaxHealth; }

	BreakableState GetBreakableState() const { return mState; }
	
private:

	void UpdateState();

	void UpdateTextureForState();

	virtual void UpdateAnimations() override;

	void SpawnDamageTransitionParticles();

	BreakableState mState;

	float mHealth;
	float mMaxHealth;

	bool mDeleteWhenBroken;
};

#endif
