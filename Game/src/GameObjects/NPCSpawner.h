#ifndef NPCSPAWNER_H
#define NPCSPAWNER_H

class NPCSpawner
{
public:
	
	virtual void SpawnNPC(const float posX, 
							const float posY, 
							bool playSoundEffect = true,
							std::string animationFile = "XmlFiles\\ninjaAnimation2.xml", 
							Vector2 & dimensions = Vector2(100.0f,100.0f),
							Vector2 & collisionDimensions = Vector2(100.0f,100.0f),
							Vector2 & collisionBoxOffset = Vector2(0.0f,0.0f)) = 0;

	virtual void SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions) = 0;

	virtual bool CanSpawnNPC() = 0;
};

#endif

