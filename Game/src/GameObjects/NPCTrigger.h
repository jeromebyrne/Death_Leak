#ifndef NPC_TRIGGER_H
#define NPC_TRIGGER_H

#include "GameObject.h"

class NPCTrigger : public GameObject
{
public:

	NPCTrigger();
	
	virtual void Update(float delta) override;

private:

	float mCooldownTime;
	float mCurrentCooldownTime;
};

#endif
