#ifndef ANIMATIONSKELETON_H
#define ANIMATIONSKELETON_H

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

	void PopulateFrameData(unsigned int frame, list<AnimationSkeletonFramePiece> framePieces);

	vector<AnimationSkeletonFramePiece> GetDataForFrame(unsigned int frame)  { return mSkeletonLines[frame]; }

	bool HasCollidedOnFrame(unsigned int frame, bool isHFlipped, Vector3 & skeletonWorldPos, Vector2 & startPoint, Vector2 & endPoint, Vector2 & intersectPointOut);

	bool HasBonesForFrame(unsigned int frame);

	void ScaleBones(float value);
	
private:

	bool Intersect(bool isHFlipped, Vector3 & skeletonWorldPos, AnimationSkeletonFramePiece & framePiece, Vector2 & otherStart, Vector2 & otherEnd, Vector2 & intersectPointOut);

	map<unsigned int, vector<AnimationSkeletonFramePiece>> mSkeletonLines;
};

#endif
