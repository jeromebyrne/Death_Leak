#ifndef CURRENCYORB_H
#define CURRENCYORB_H

#include "solidmovingsprite.h"

class CurrencyOrb : public SolidMovingSprite
{
public:

	enum OrbState
	{
		kIdle,
		kTracking,
		kSpawnPeriod
	};

	CurrencyOrb(void);
	virtual ~CurrencyOrb(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;

	virtual void Update(float delta) override;

	static void SpawnOrbs(Vector3 & position, unsigned int numOrbs);

private:

	void DoIdleHover(float delta);

	void DoTrackPlayer(float delta);

	void AddTrailParticles();

	void DoCollisionSmallType(Player * player);

	void DoCollisionLargeType(Player * player);

	OrbState mCurrentState;

	static unsigned long mLastTimePlayedSFX;

	bool mIsLargeType;

	int mParticleTrailObjectId;

	float mSpawnPeriodTime;

	float mTimeTracking;
};

#endif