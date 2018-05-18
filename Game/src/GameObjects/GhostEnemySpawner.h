#ifndef GHOSTENEMYSPAWNER_H
#define GHOSTENEMYSPAWNER_H

#include "npcspawner.h"

class GhostEnemySpawner : public NPCSpawner
{
public:
	GhostEnemySpawner(void);
	virtual ~GhostEnemySpawner(void);

	virtual void SpawnNPC(const float posX, 
							const float posY, 
							bool playSoundEffect = true,
							std::string animationFile = "XmlFiles\\ninjaAnimation2.xml", 
							Vector3 & dimensions = Vector3(100, 100, 0),
							Vector3 & collisionDimensions = Vector3(100, 100, 0),
							Vector2 & collisionBoxOffset = Vector2(0,0)) override;

	virtual void SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions) override;
};

#endif

