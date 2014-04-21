#ifndef NPCSPAWNER_H
#define NPCSPAWNER_H

class NPCSpawner
{
public:
	
	virtual void SpawnNPC(const float posX, const float posY, bool playSoundEffect = true, std::string animationFile = "XmlFiles\\ninjaAnimation2.xml") = 0;

	virtual void SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions) = 0;
};

#endif

