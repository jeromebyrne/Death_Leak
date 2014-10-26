#ifndef CHARACTER_H
#define CHARACTER_H
#include "solidmovingsprite.h"
#include "animation.h"
#include <set>

class Projectile;

class Character : public SolidMovingSprite
{

public:

	Character(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~Character(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void OnCollision(SolidMovingSprite * object) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	inline void SetMaxJumpSpeed(float value)
	{
		m_maxJumpSpeed = value;
	}
	virtual void Jump(float percent);
	virtual void WallJump(float percent);
	virtual void AccelerateX(float directionX);
	virtual void OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;

	void setAccelXRate(const float accelRate) { mAccelXRate = accelRate; }

	float GetHealth() const { return mHealth; }
	float GetMaxHealth() const { return mMaxHealth; }

	void IncreaseHealth(float value);

	virtual void DebugDraw(ID3D10Device *  device);

	void SetSprintActive(bool value);
	bool GetIsSprintActive() { return mSprintActive; }

	virtual Projectile * FireWeapon(Vector2 direction) = 0;
	virtual Projectile * FireBomb(Vector2 direction) = 0;

	virtual void DoMeleeAttack();

protected:

	void UpdateFootsteps(SolidMovingSprite * solidObject);

	void UpdateWaterWadeSFX();

	bool m_isJumping;
	float m_maxJumpSpeed;
	float m_lastTimePlayedFootstep; // the last time in ms when we played a footstep sound
	float m_lastTimePlayedWaterWadeSFX; 
	float m_footstepTime; // the time in between each footstep
	float m_sprintFootstepTime; // the time between each footstep when sprinting
	float m_waterWadeSFXTime;
	virtual void UpdateAnimations() override; // override sprite update animations
	float mAccelXRate;
	float mHealth;
	float mMaxHealth;
	float mSprintVelocityX;
	float mDefaultVelocityX; // max velocity when not sprinting
	bool mSprintActive; // are we sprinting?
	bool mHasExploded;
	ID3D10ShaderResourceView * m_mainBodyTexture;
	ID3D10ShaderResourceView * m_armTexture;
	Vector2 m_projectileOffset; // where does a projectile fire from in relation to the centre?

	std::string mProjectileFilePath;
	std::string mProjectileImpactFilePath;

	void PlayRandomWeaponFireSound();

	static float mLastTimePlayedDeathSFX;
	float mLastTimePlayedDamageSound;
	const float mDamageSoundDelayMilli;

	float mRunAnimFramerateMultiplier;

	bool mPlayFootsteps;

	bool mMatchAnimFrameRateWithMovement;
};

#endif
