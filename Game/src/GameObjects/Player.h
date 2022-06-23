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
	virtual Projectile * FireWeapon(Vector2 direction, float speedMultiplier) override;
	virtual Projectile * FireBomb(Vector2 direction) override;
	void ResetProjectileFireDelay();
	void Draw(ID3D10Device* device, Camera2D* camera) override;

	void AddAimLineSprite();
	void SetAimLineDirection(Vector2 & dir);

	void SetCurrentSolidLineDirection(const Vector2 & direction);

	Vector2 GetCurrentSolidLineDirection() const { return mCurrentSolidLineDirection; }

	bool CanJump() const override;

	float GetMaxFocusAmount() const { return mMaxFocusAmount; }

	float GetCurrentFocusAmount() const { return mCurrentFocusAmount; }

	void SetCurrentFocusAmount(float value) { mCurrentFocusAmount = value; }

	void SetMaxFocus(float value) { mMaxFocusAmount = value; }

	void TryFocus();

	void StopFocus();

	void TrySprint();

	void StopSprint();

	bool CanSprint();

	void UpdateResistance();

	bool HasEnoughFocus(float amountToUse);

	virtual bool DoMeleeAttack() override;

	virtual bool DoDownwardDash() override;

	void ConsumeFocus(float focusAmount);

	virtual bool Roll() override;

	void SetMaxHealth(float value);

	void SetHealth(float value);

	void UpdateAnimations() override;

	bool CanBeControlled();

	bool IsInFinalLevel() {
		return mIsInFinalLevel;
	}

	void SetUpgradedKnifeTexture();

	void OnPurchaseSpeedUpgrade();

private:

	void UpdateIsPullingSwordFromStomach(float delta);

	virtual void DebugDraw(ID3D10Device *  device) override;

	void CheckForAndDoLevelUp();

	void UpdateFocus(float delta);

	void TriggerDiedUI();

	void EndStory();

	void InitialiseFinalLevelText();

	float mProjectileFireDelay;
	float mTimeUntilProjectileReady;

	unsigned mFireBurstNum;
	unsigned int mCurrentBurstNum;
	float mFireBurstDelay;
	float mTimeUntilFireBurstAvailable;
	bool mBurstFireEnabled;

	float mMaxFocusAmount = 100.0f;
	float mCurrentFocusAmount = mMaxFocusAmount;
	float mCurrentFocusCooldown = 0.0f;
	bool mIsFocusing = false;

	Sprite * mAimLineSprite;

	float mTimeUntilAimLineStartDisappearing;

	Vector2 mCurrentSolidLineDirection;

	bool mHasTriggeredDiedUI = false;

	float mCameraZoomOnLoad = 1.0f;

	bool mIsDoingSprintZoom = false;

	float mCurrentTimePullingSword = 0.0f;

	float mTotalTimePullingSword = 0.0f;

	irrklang::ISound* mBreathingIntroSFX = nullptr;

	bool mIsInFinalLevel = false;

	float mCurrentTimeInFinalLevel = 0.0f;

	bool mHasDisplayedFinalLevelText = false;

	ID3DX10Font* mFinalLevelTitleText = nullptr;
};

#endif
