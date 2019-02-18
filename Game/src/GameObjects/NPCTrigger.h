#ifndef NPC_TRIGGER_H
#define NPC_TRIGGER_H

#include "GameObject.h"

class NPCTrigger : public GameObject
{
public:

	NPCTrigger();
	
	virtual void Update(float delta) override;

	enum NpcType
	{
		kUnknown,
		kNinja,
		kGhost,
		kSkeleton
	};

private:

	void XmlRead(TiXmlElement * element) override;
	void XmlWrite(TiXmlElement * element) override;

	NpcType GetNPCTypeFromString(const string & typeAsString);
	string GetNPCStringFromType(NpcType);

	virtual void NPCTrigger::DebugDraw(ID3D10Device *  device) override;
	
	void SpawnEnemies(Player * player);
	void SpawnNinjas(Player * player);
	void SpawnGhosts(Player * player);
	void SpawnSkeletons(Player * player);

	void ShowSpawnIcon();

	float mCooldownTime;
	float mCurrentCooldownTime;
	unsigned int mNumEnemies;

	NpcType mType = kNinja;
};

#endif
