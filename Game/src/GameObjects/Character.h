#ifndef CHARACTER_H
#define CHARACTER_H
#include "solidmovingsprite.h"
#include "animation.h"
#include <set>

class Projectile;
class ParticleSpray;

class Character : public SolidMovingSprite
{
public:

	enum CurrentMeleePhase
	{
		kMeleePhase1,
		kMeleePhase2,
		kMeleePhase3,
		kMeleeFinish
	};

	Character(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10.0f);
	virtual ~Character(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	inline void SetMaxJumpSpeed(float value) { m_maxJumpSpeed = value; }
	virtual bool Jump(float percent);
	bool WaterJump();
	void IncreaseJump(float percent);
	virtual void WallJump(int directionX, float percent);
	virtual bool Roll();
	virtual void AccelerateX(float directionX, float percent = 1.0f);
	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) override;
	void setAccelXRate(const float accelRate) { mAccelXRate = accelRate; }
	float GetHealth() const { return mHealth; }
	float GetMaxHealth() const { return mMaxHealth; }
	void IncreaseHealth(float value);
	virtual void DebugDraw(ID3D10Device *  device);
	void SetSprintActive(bool value);
	bool GetIsSprintActive() { return mSprintActive; }
	virtual Projectile * FireWeapon(Vector2 direction, float speedMultiplier) = 0;
	virtual Projectile * FireBomb(Vector2 direction) = 0;
	virtual bool DoMeleeAttack();
	unsigned GetMaxJumpsAllowed() const { return mMaxJumpsAllowed; }
	unsigned GetCurrentJumpsBeforeLand() const { return mCurrentJumpsBeforeLand; }
	float GetTimeNotOnSolidSurface() const { return mTimeNotOnSolidSurface; }
	float GetTimeOnSolidSurface() const { return mTimeOnSolidSurface; }
	void SetCurrentSolidLineDroppingDownThroughId(unsigned int value) { mCurrentSolidLineDroppingDownThroughId = value; }
	unsigned GetCurrentSolidLineDroppingDownThroughId() const { return mCurrentSolidLineDroppingDownThroughId; }
	void DropDown();

	bool IsStrafing() const  { return mIsStrafing; }
	void SetIsStrafing(bool value) { mIsStrafing = value; }
	float GetStrafeDirectionX() const { return mStrafeDirectionX; }
	void SetStrafeDirectionX(float value) { mStrafeDirectionX = value; }

	virtual bool CanTeleport() const { return true; }

	void Teleport(float posX, float posY, bool showParticles);

	void SetCrouching(bool value);

	bool JustFellFromLargeDistance() const { return mJustfellFromLargeDistance; }

	bool JustFellFromShortDistance() const { return mJustFellFromShortDistance;  }

	bool IsWallJumping() const { return mIsWallJumping; }

	float GetCurrentWallJumpXDirection() const { return mCurrentWallJumpXDirection; }

	bool GetIsRolling() const { return mIsRolling; }

	bool IsDoingMelee() const { return mIsDoingMelee; }

	bool IsCrouching() const { return mIsCrouching; }

	void DoAnimationEffectIfApplicable(AnimationPart * bodyPart);

	CurrentMeleePhase GetCurrentMeleePhase() const { return mCurrentMeleePhase; }

	bool WillDeflectProjectile(float projectileDirectionX, float projectileCollisionLeft, float projectileCollisionRight);

	virtual bool DoDownwardDash();

	bool GetIsDownwardDashing() const { return mIsDownwardDashing; }

	bool CanIncreaseJumpIntensity() const { return mCanIncreaseJumpVelocity && m_velocity.Y >= 0.0f; }

	virtual bool CanJump() const;

	virtual bool CanRoll() const { return !WasInWaterLastFrame(); }

	// This is used for the animation viewer
	virtual void UpdateAnimTexture(const string & bodyPart) override;

	void Stun(float stunTime);

	bool IsStunned() { return mCurrentStunTime > 0.0f; }

	bool IsFullyCrouched() { return mIsFullyCrouched; }

	bool IsDead();

	void ResetJumpHeightVariables();

protected:

	void SetIsWallJumping(bool value);
	virtual void UpdateAnimations() override; // override sprite update animations
	void PlayRandomWeaponFireSound();

	void DoLargeImpactLanding();

	void DoMeleeCollisions(SolidMovingSprite * object);

	void AddStunParticles();

	void EnableStunParticles(bool enabled);

	void UpdateCollisionBox();

	void FireBloodSpatter(Vector2 direction, const Vector2 & origin);

	void DoLandOnSolidSurfaceEffects(float dropDistance);

	void DoJumpParticles();

	void FinishTeleport(float posX, float posY, bool showParticles);

	bool IsTeleporting() { return mTeleportCurrentTime > 0.0f; }

	bool m_isJumping;
	float m_maxJumpSpeed;
	float m_lastTimePlayedWaterWadeSFX; 
	float m_waterWadeSFXTime;
	float mAccelXRate;
	float mHealth;
	float mMaxHealth;
	float mSprintVelocityX;
	float mDefaultVelocityX; // max velocity when not sprinting
	bool mSprintActive; // are we sprinting?
	bool mHasExploded;
	bool mExplodesGruesomely;
	ID3D10ShaderResourceView * m_mainBodyTexture;
	ID3D10ShaderResourceView * m_armTexture;
	Vector2 m_projectileOffset; // where does a projectile fire from in relation to the centre?
	std::string mProjectileFilePath;
	std::string mProjectileImpactFilePath;
	static float mLastTimePlayedDeathSFX;
	float mLastTimePlayedDamageSound;
	const float mDamageSoundDelayMilli;
	float mRunAnimFramerateMultiplier;
	bool mMatchAnimFrameRateWithMovement;
	bool mIsMidAirMovingDown;
	bool mIsMidAirMovingUp;
	float mMidAirMovingUpStartTime;
	float mMidAirMovingDownStartTime;
	unsigned mMaxJumpsAllowed;
	unsigned mCurrentJumpsBeforeLand;
	float mTimeNotOnSolidSurface;
	float mTimeOnSolidSurface;
	unsigned int mCurrentSolidLineDroppingDownThroughId;
	bool mIsStrafing;
	float mStrafeDirectionX;
	bool mIsCrouching;
	bool mWasCrouching;
	bool mIsFullyCrouched;
	float mHighestPointWhileInAir;
	bool mJustFellFromShortDistance;
	bool mJustfellFromLargeDistance;
	unsigned mLastRunFramePlayed;
	bool mIsWallJumping;
	float mWallJumpCountdownTime;
	float mCurrentWallJumpXDirection;
	bool mIsRolling;
	bool mDoReboundJump;
	bool mDoSwimBurstAnim;
	bool mIsDoingMelee;
	bool mIsDownwardDashing;
	bool mWasDownwardDashing;
	bool mCanIncreaseJumpVelocity;
	float mCurrentStunTime = 0.0f;
	ParticleSpray * mStunParticles;
	Vector2 mRegularCollisionBox;
	Vector2 mCollisionBoxOffsetOriginal;
	float mMeleeCollisionBoundsX = 1.5f;
	Vector2 mRegularSpriteSize;
	Vector2 mMeleeSpriteSize;
	bool mDoMeleeSpriteResize = false;
	bool mEmitsBlood = true;
	static float mTeleportSfxDelay;

	CurrentMeleePhase mCurrentMeleePhase = kMeleePhase1;

	float mMeleeDamage = 500.0f;

	float mTeleportCurrentTime = 0.0f;
	Vector2 mTeleportPosition;
	bool mShowTeleportParticles = false;
};

#endif
