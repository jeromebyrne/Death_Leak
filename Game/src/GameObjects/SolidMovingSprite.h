#ifndef SOLIDMOVINGSPRITE_H
#define SOLIDMOVINGSPRITE_H
#include "movingsprite.h"

class SolidLineStrip;

class SolidMovingSprite : public MovingSprite
{
public:

	SolidMovingSprite(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0, float groundFriction = 1, float airResistance = 1);
	virtual ~SolidMovingSprite(void);
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual bool OnCollision(SolidMovingSprite * object);
	virtual void Update(float delta) override;
	virtual void SetNotColliding() // sets properties that mean we are not colliding
	{
		m_onTopOfOtherSolidObject = false;
		m_collidingAtSideOfObject = false;
		mObjectMovingWith = 0;
	}

	Vector2 CollisionBoxOffset() const { return mCollisionBoxOffset; }
	Vector3 CollisionDimensions() const { return m_collisionBoxDimensions; }

	void SetCollisionDimensions(Vector3 value) { m_collisionBoxDimensions = value; }
	void SetCollisionBoxOffset(Vector2 value) { mCollisionBoxOffset = value; }

	bool GetIsCollidingAtObjectSide() const { return m_collidingAtSideOfObject; }
	bool GetIsCollidingOnTopOfObject() const { return m_onTopOfOtherSolidObject; }
	void SetIsCollidingOnTopOfObject(bool value) { m_onTopOfOtherSolidObject = value; }
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;

	// the left side of our bounding box
	inline float CollisionLeft() const
	{
		return (m_position.X - (m_collisionBoxDimensions.X * 0.5f)) + mCollisionBoxOffset.X;
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
		return (m_position.Y - (m_collisionBoxDimensions.Y * 0.5)) + mCollisionBoxOffset.Y;
	}

	inline float CollisionBottomLastFrame() const
	{
		return (m_lastPosition.Y - (m_collisionBoxDimensions.Y * 0.5)) + mCollisionBoxOffset.Y;
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

	virtual void DebugDraw(ID3D10Device *  device) override;

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;

	virtual void SetupDebugDraw() override;

	virtual bool IsOnSolidLine() const { return mIsOnSolidLine; }

	virtual void SetIsOnSolidLine(bool value, SolidLineStrip * lineStrip);

	bool IsOnSolidSurface();

	SolidLineStrip * GetCurrentSolidLineStrip() const { return mCurrentSolidLineStrip; }
	
protected:

	virtual void DoWaterAccelerationBubbles() override;

	bool m_passive; // is this an active collision object or passive?

	Vector3 m_collisionBoxDimensions;
	bool m_markedForResolve; // do we need to do collision resolution
	bool m_onTopOfOtherSolidObject; // are we colliding above another object (moving on top of it scenario)
	bool m_collidingAtSideOfObject; // we are colliding at either side of another solid object
	
	bool m_applyDamage; // does this obejct apply damage to other objects that hit it?
	float m_applyDamageAmount;

	bool mBouncable; // does this object bounce off other objects when it collides
	float mBounceDampening;

	Vector2 mCollisionBoxOffset;

	bool mCanBeDamaged;

	bool mIsOnSolidLine;

private:

	VertexPositionColor m_collisionBoxVertices[12];
	// create a vertex buffer
	ID3D10Buffer* m_collisionBoxVBuffer;

	SolidLineStrip * mCurrentSolidLineStrip;
};

#endif
