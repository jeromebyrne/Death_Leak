#ifndef ANIMATION_H
#define ANIMATION_H

#include "AnimationPart.h"
#include "AnimationSkeleton.h"

class Animation
{
public:

	Animation(const char* filename);
	~Animation(void);

	AnimationPart* GetPart(const string & partName);
	void SetPartSequence(const string & partName, const string & sequenceName);

	AnimationSkeleton * GetSkeletonForCurrentSequence(const string & bodyPart);

	std::vector<std::string> GetSequenceNamesForBodyPart(const std::string & bodyPart);

private:

	map<string, AnimationPart*> m_animationParts;

	void ReadXml(const char* filename);
};

#endif
