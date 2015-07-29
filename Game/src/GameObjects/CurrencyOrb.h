#ifndef CURRENCYORB_H
#define CURRENCYORB_H

#include "solidmovingsprite.h"

class CurrencyOrb : public SolidMovingSprite
{
public:

	enum OrbState
	{
		kIdle,
		kTracking
	};

	CurrencyOrb(void);
	virtual ~CurrencyOrb(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;

	virtual void Update(float delta) override;

private:

	void DoIdleHover(float delta);

	void DoTrackPlayer(float delta);

	OrbState mCurrentState;
};

#endif