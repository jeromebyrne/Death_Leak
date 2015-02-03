#ifndef NPC_TRIGGER_H
#define NPC_TRIGGER_H

#include "GameObject.h"

class NPCTrigger : public GameObject
{
public:

	NPCTrigger();
	
	virtual void Update(float delta) override;

private:

	void XmlRead(TiXmlElement * element) override;
	void XmlWrite(TiXmlElement * element) override;

	float mCooldownTime;
	float mCurrentCooldownTime;
	unsigned int mNumEnemies;
};

#endif
