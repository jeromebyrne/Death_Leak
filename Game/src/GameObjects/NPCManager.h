#ifndef NPCMANAGER_H
#define NPCMANAGER_H
#include "NPC.h"

class NPCManager
{
private:
	NPCManager(void);
	virtual ~NPCManager(void);

	static NPCManager * mInstance;

	list<NPC *> m_npcList;

	void ResolveCollisions(NPC * npc, NPC * otherNPC);

public:

	static NPCManager * Instance();

	void AddNPC(NPC * npc);

	void RemoveNPC(NPC * npc);

	void Update();

	list<NPC*> GetNPCList() const { return m_npcList; } 
};

#endif
