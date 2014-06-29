#ifndef NPC_H
#define NPC_H

#include "player.h"
#include "AIstate.h"
#include <set>

class AIStateFollow;

class NPC : public Character
{
private:
	AIState * m_currentState;
	std::set<NPC *> m_resolvedNPCCollisionSet;
protected:
	Player * m_player;
	AIStateFollow * m_friendlyFollowState;
	bool mCheckNPCOverlapCollisions;
public:
	
	// let the state classes access private data as they are an extension
	// of the NPC class 
	friend class AIStateFollow;

	NPC(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~NPC(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	inline void SetPlayer(Player * player) { m_player = player; }

	void SetState(AIState::AIStateType state);

	void FireProjectileAtObject(GameObject * target);

	virtual void OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;

	virtual Projectile * FireWeapon(Vector2 direction) override; 

	virtual Projectile * FireBomb(Vector2 direction) override { GAME_ASSERT(false); return nullptr; }

	void AddToNPCResolvedList(NPC * otherNPC ) { m_resolvedNPCCollisionSet.insert(otherNPC); } 

	bool NPCCollisionSetContains(NPC * npc) { return m_resolvedNPCCollisionSet.find(npc) != m_resolvedNPCCollisionSet.end(); }

	void ClearNPCCollisionSet() { m_resolvedNPCCollisionSet.clear();  }

	bool GetCheckNPCOverlapCollisions() { return mCheckNPCOverlapCollisions; }
};

#endif
