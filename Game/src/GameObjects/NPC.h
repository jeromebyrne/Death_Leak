#ifndef NPC_H
#define NPC_H

#include "player.h"
#include "AIstate.h"
#include <set>

class AIStateFollow;
class AIStateRepel;

class NPC : public Character
{
public:

	// let the state classes access private data as they are an extension
	// of the NPC class 
	friend class AIStateFollow;
	friend class AIStateRepel;
	friend class AIStateRangeAttack;
	friend class AIStateButterflyWander;
	friend class AIStateGroundAnimalWander;
	friend class AIStateFlying;

	NPC(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10.0f);
	virtual ~NPC(void);
	virtual void Update(float delta) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void Initialise() override;

	inline void SetPlayer(Player * player) { m_player = player; }

	void SetState(AIState::AIStateType state);

	virtual void FireProjectileAtObject(GameObject * target);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) override;

	virtual Projectile * FireWeapon(Vector2 direction) override;

	virtual Projectile * FireBomb(Vector2 direction) override { GAME_ASSERT(false); return nullptr; }

	void AddToNPCResolvedList(NPC * otherNPC) { m_resolvedNPCCollisionSet.insert(otherNPC); }

	bool NPCCollisionSetContains(NPC * npc) { return m_resolvedNPCCollisionSet.find(npc) != m_resolvedNPCCollisionSet.end(); }

	void ClearNPCCollisionSet() { m_resolvedNPCCollisionSet.clear(); }

	bool GetCheckNPCOverlapCollisions() { return mCheckNPCOverlapCollisions; }

	bool IsPlayerEnemy() const { return mIsPlayerEnemy; }

	void SetIsPlayerEnemy(bool value) { mIsPlayerEnemy = value; }

protected:

	Player * m_player;
	AIStateFollow * m_friendlyFollowState;
	AIStateRepel * m_repelState;
	AIStateRangeAttack * m_rangeAttackState;
	AIStateButterflyWander * m_butterflyWander;
	AIStateGroundAnimalWander * mGroundAnimalWanderState;
	AIStateFlying * mFlyingState;
	bool mCheckNPCOverlapCollisions;
	bool mAddHealthBar;

protected:

	void AddHealthBar();
	void UpdateHealthBar(float delta);

	virtual void DebugDraw(ID3D10Device *  device) override;
	virtual void SetupDebugDraw() override;

	void DoNinjaExplosion();

	void SpawnLimb(const string & filename, bool attachParticles, float speedMultiplier);

	AIState * m_currentState;
	std::set<NPC *> m_resolvedNPCCollisionSet;
	float mLastFireTime;
	float mNextFireTime;

	Sprite * mHealthBarSprite;
	Sprite * mHealthBarUnderlaySprite;
	Sprite * mHealthBarOverlaySprite;
	float mCurrentHealthMeterScale;
	float mHealthMeterHealthBeforeDecrease;
	bool mIsPlayerEnemy;
	int mRandHealthBarOffsetY;
	int mDamageInARowCount = 0;
	float mDamageInARowCountdown = 0.0f;
	int mFireProjectileRandOffsetMax = 250;
};

#endif
