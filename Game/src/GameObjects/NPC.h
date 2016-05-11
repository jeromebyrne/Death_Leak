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

	NPC(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~NPC(void);
	virtual void Update(float delta) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void Initialise() override;

	inline void SetPlayer(Player * player) { m_player = player; }

	void SetState(AIState::AIStateType state);

	void FireProjectileAtObject(GameObject * target);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;

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
	bool mCheckNPCOverlapCollisions;
	bool mAddHealthBar;

private:

	void AddHealthBar();
	void UpdateHealthBar(float delta);

	virtual void DebugDraw(ID3D10Device *  device) override;
	virtual void SetupDebugDraw() override;

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
};

#endif
