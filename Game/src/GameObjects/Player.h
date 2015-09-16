#ifndef PLAYER_H
#define PLAYER_H
#include "character.h"

class Player : public Character
{

public:

	Player(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~Player(void);
	virtual void Update(float delta) override;
	void Initialise() override;
	void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;
	virtual Projectile * FireWeapon(Vector2 direction) override;
	virtual Projectile * FireBomb(Vector2 direction) override;
	void ResetProjectileFireDelay();

private:

	virtual void DebugDraw(ID3D10Device *  device) override;

	void CheckForAndDoLevelUp();

	float mProjectileFireDelay;
	float mTimeUntilProjectileReady;

	unsigned mFireBurstNum;
	unsigned int mCurrentBurstNum;
	float mFireBurstDelay;
	float mTimeUntilFireBurstAvailable;

};

#endif
