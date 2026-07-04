#ifndef ANIMATIONSKELETON_H
#define ANIMATIONSKELETON_H

#include "Vector2.h"

#include <list>
#include <map>
#include <vector>

class AnimationSkeleton
{
public:

	struct AnimationSkeletonFramePiece
	{
		Vector2 mStartPos;
		Vector2 mEndPos;
		float mLength;
		Vector2 mLineDirection;
		Vector2 mNormal;
	};

	AnimationSkeleton();
	virtual ~AnimationSkeleton();

	void PopulateFrameData(unsigned int frame, std::list<AnimationSkeletonFramePiece> framePieces);

	std::vector<AnimationSkeletonFramePiece> GetDataForFrame(unsigned int frame)  { return mSkeletonLines[frame]; }

	bool HasCollidedOnFrame(unsigned int frame, bool isHFlipped, Vector2 & skeletonWorldPos, Vector2 & startPoint, Vector2 & endPoint, Vector2 & intersectPointOut);

	bool HasBonesForFrame(unsigned int frame);

	void ScaleBones(float value);
	
private:

	bool Intersect(bool isHFlipped, Vector2 & skeletonWorldPos, AnimationSkeletonFramePiece & framePiece, Vector2 & otherStart, Vector2 & otherEnd, Vector2 & intersectPointOut);

	std::map<unsigned int, std::vector<AnimationSkeletonFramePiece>> mSkeletonLines;
};

#endif
