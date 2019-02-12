#ifndef SOLIDMOVINGSPRITE_H
#define SOLIDMOVINGSPRITE_H
#include "movingsprite.h"

class SolidLineStrip;

class SolidMovingSprite : public MovingSprite
{
public:

	SolidMovingSprite(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10.0f, float groundFriction = 1.0f, float airResistance = 1.0f);
	virtual ~SolidMovingSprite(void);
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual bool OnCollision(SolidMovingSprite * object);
	virtual void Update(float delta) override;
	virtual void SetNotColliding() // sets properties that mean we are not colliding
	{
		m_onTopOfOtherSolidObject = false;
		m_collidingAtLeftSideOfObject = false;
		m_collidingAtRightSideOfObject = false;
	}

	Vector2 CollisionBoxOffset() const { return mCollisionBoxOffset; }
	Vector2 CollisionDimensions() const { return m_collisionBoxDimensions; }

	void SetCollisionDimensions(Vector2 value) { m_collisionBoxDimensions = value; }
	void SetCollisionBoxOffset(Vector2 value) { mCollisionBoxOffset = value; }

	bool GetIsCollidingAtObjectSide() const { return m_collidingAtLeftSideOfObject || m_collidingAtRightSideOfObject; }
	bool GetIsCollidingOnTopOfObject() const { return m_onTopOfOtherSolidObject; }
	void SetIsCollidingOnTopOfObject(bool value) { m_onTopOfOtherSolidObject = value; }
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;

	// the left side of our bounding box
	inline float CollisionLeft() const
	{
		return (X() - (m_collisionBoxDimensions.X * 0.5f)) + mCollisionBoxOffset.X;
	}
	// the right side of our bounding box
	inline float CollisionRight() const
	{
		return (m_position.X + (m_collisionBoxDimensions.X * 0.5f)) + mCollisionBoxOffset.X;
	}
	// the top of our bounding box
	inline float CollisionTop() const
	{
		return (m_position.Y + (m_collisionBoxDimensions.Y * 0.5f)) + mCollisionBoxOffset.Y;
	}
	// the bottom of our bounding box
	inline float CollisionBottom() const
	{
		return (m_position.Y - (m_collisionBoxDimensions.Y * 0.5f)) + mCollisionBoxOffset.Y;
	}

	inline float CollisionBottomLastFrame() const
	{
		return (m_lastPosition.Y - (m_collisionBoxDimensions.Y * 0.5f)) + mCollisionBoxOffset.Y;
	}

	inline float CollisionCentreX() const
	{
		return m_position.X + mCollisionBoxOffset.X;
	}

	inline float CollisionCentreY() const
	{
		return m_position.Y + mCollisionBoxOffset.Y;
	}

	inline bool IsPassive()
	{
		return m_passive;
	}

	void SetPassive(bool value) { m_passive = value; }

	virtual void DebugDraw(ID3D10Device *  device) override;

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) override;

	virtual void SetupDebugDraw() override;

	virtual bool IsOnSolidLine() const { return mIsOnSolidLine; }

	virtual void SetIsOnSolidLine(bool value, SolidLineStrip * lineStrip);

	bool IsOnSolidSurface() const override;

	SolidLineStrip * GetCurrentSolidLineStrip() const { return mCurrentSolidLineStrip; }

	bool CanBeStruckByMelee() const { return mCanBeDamaged && mMeleeStrikeCooldown <= 0.0f; }

	void TriggerMeleeCooldown();

	bool IsBouncy() const { return mIsBouncy; }
	float GetBounceMultiplier() const { return mBounceMultiplier; }

	virtual void PostUpdate(float delta) override;

	bool IsOnSolidlineThisFrame() { return mIsOnSolidlineThisFrame; }

	bool IsBouncable() const { return mBouncable;  }

	float GetBounceDampening() const { return mBounceDampening; }

	void SetBouncable(bool value) { mBouncable = value; }

	void SetBounceDampening(float value) { mBounceDampening = value; }
	
protected:

	virtual void DoWaterAccelerationBubbles() override;

	bool m_passive; // is this an active collision object or passive?

	Vector2 m_collisionBoxDimensions;
	bool m_markedForResolve; // do we need to do collision resolution
	bool m_onTopOfOtherSolidObject; // are we colliding above another object (moving on top of it scenario)
	bool m_collidingAtLeftSideOfObject;
	bool m_collidingAtRightSideOfObject;
	
	bool m_applyDamage; // does this obejct apply damage to other objects that hit it?
	float m_applyDamageAmount;

	bool mBouncable; // does this object bounce off other objects when it collides
	float mBounceDampening;

	Vector2 mCollisionBoxOffset;

	bool mCanBeDamaged;

	bool mIsOnSolidLine;

	bool mIsBouncy = false;
	float mBounceMultiplier = 1.3f;

private:

	VertexPositionColor m_collisionBoxVertices[12];
	// create a vertex buffer
	ID3D10Buffer* m_collisionBoxVBuffer;

	SolidLineStrip * mCurrentSolidLineStrip;

	float mMeleeStrikeCooldown = 0.0f;

	bool mIsOnSolidlineThisFrame = false;
};

#endif
