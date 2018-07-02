#ifndef PLAYER_H
#define PLAYER_H
#include "character.h"

class Player : public Character
{
public:

	Player(float x = 0, float y = 0, float width = 10, float height = 10);
	virtual ~Player(void);
	virtual void Update(float delta) override;
	void Initialise() override;
	void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) override;
	virtual Projectile * FireWeapon(Vector2 direction) override;
	virtual Projectile * FireBomb(Vector2 direction) override;
	void ResetProjectileFireDelay();

	void AddAimLineSprite();
	void SetAimLineDirection(Vector2 & dir);

	void SetCurrentSolidLineDirection(const Vector2 & direction);

	Vector2 GetCurrentSolidLineDirection() const { return mCurrentSolidLineDirection; }

	bool CanJump() const override;

	float GetMaxFocusAmount() const { return mMaxFocusAmount; }

	float GetCurrentFocusAmount() const { return mCurrentFocusAmount; }

	void TryFocus();

	void StopFocus();

	void UpdateResistance();

	bool HasEnoughFocusForMelee();

	bool HasEnoughFocusForDownwardDash();

	virtual bool DoMeleeAttack() override;

	virtual bool DoDownwardDash() override;

	void ConsumeFocus(float focusAmount);

private:

	virtual void DebugDraw(ID3D10Device *  device) override;

	void CheckForAndDoLevelUp();

	void UpdateFocus(float delta);

	float mProjectileFireDelay;
	float mTimeUntilProjectileReady;

	unsigned mFireBurstNum;
	unsigned int mCurrentBurstNum;
	float mFireBurstDelay;
	float mTimeUntilFireBurstAvailable;
	bool mBurstFireEnabled;

	float mMaxFocusAmount = 100.0f;
	float mCurrentFocusAmount = 100.0f;
	float mCurrentFocusCooldown = 0.0f;
	bool mIsFocusing = false;

	Sprite * mAimLineSprite;

	float mTimeUntilAimLineStartDisappearing;

	Vector2 mCurrentSolidLineDirection;
};

#endif
