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
	
private:

	map<unsigned int, vector<AnimationSkeletonFramePiece>> mSkeletonLines;
};

#endif
