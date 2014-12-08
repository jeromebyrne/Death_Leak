#ifndef ANIMATION_H
#define ANIMATION_H

#include "AnimationPart.h"
#include "AnimationSkeleton.h"

class Animation
{
private:

	map<string,AnimationPart*> m_animationParts;

	void ReadXml(const char* filename);

public:

	Animation(const char* filename);
	~Animation(void);

	AnimationPart* GetPart(string partName);
	void SetPartSequence(string partName, string sequenceName);

	AnimationSkeleton * GetSkeletonForCurrentSequence(const char * bodyPart);
};

#endif
