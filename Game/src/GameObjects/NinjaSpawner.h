#ifndef NINJASPAWNER_H
#define NINJASPAWNER_H

#include "npcspawner.h"
class NinjaSpawner : public NPCSpawner
{
public:
	NinjaSpawner(void);
	virtual ~NinjaSpawner(void);

	virtual void SpawnNPC(const float posX, const float posY, bool playSoundEffect = true, std::string animationFile = "XmlFiles\\ninjaAnimation2.xml") override;

	virtual void SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions) override;
};

#endif

