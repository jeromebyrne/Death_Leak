#ifndef ANIMATION_H
#define ANIMATION_H

#include "AnimationPart.h"
#include "AnimationSkeleton.h"

#include <map>
#include <string>
#include <vector>

class Animation
{
public:

	Animation(const char* filename);
	~Animation(void);

	AnimationPart* GetPart(const std::string & partName);
	void SetPartSequence(const std::string & partName, const std::string & sequenceName);

	AnimationSkeleton * GetSkeletonForCurrentSequence(const std::string & bodyPart);

	std::vector<std::string> GetSequenceNamesForBodyPart(const std::string & bodyPart);

	void ScaleSkeleton(float value);

	// This function is needed for viewing animations
	void JumpToNextFrame(const std::string & bodyPart);

	// This function is needed for viewing animations
	void JumpToPreviousFrame(const std::string & bodyPart);

	// This function is needed for viewing animations
	void JumpToNextSequence(const std::string & bodyPart);

	// This function is needed for viewing animations
	void JumpToPreviousSequence(const std::string & bodyPart);

	// used for animation viewer
	int CurrentFrame(const std::string & bodyPart);

	const std::string & CurrentSequenceName(const std::string & bodyPart);

private:

	std::map<std::string, AnimationPart*> m_animationParts;

	void ReadXml(const char* filename);
};

#endif
