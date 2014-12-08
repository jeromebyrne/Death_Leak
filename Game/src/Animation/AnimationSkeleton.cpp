#include "precompiled.h"
#include "AnimationSkeleton.h"

AnimationSkeleton::AnimationSkeleton()
{

}

AnimationSkeleton::~AnimationSkeleton()
{
}

void AnimationSkeleton::PopulateFrameData(unsigned int frame, list<AnimationSkeletonFramePiece> framePieces)
{
	mSkeletonLines[frame].clear();
	mSkeletonLines[frame].reserve(framePieces.size());

	for (auto & piece : framePieces)
	{
		AnimationSkeletonFramePiece internalPiece;
		internalPiece.mStartPos = piece.mStartPos;
		internalPiece.mEndPos = piece.mEndPos;

		Vector2 lineDirection = piece.mEndPos - piece.mStartPos;
		internalPiece.mLength = lineDirection.Length();

		if (internalPiece.mLength <= 0)
		{
			GAME_ASSERT(false);
			continue;
		}

		internalPiece.mLineDirection.X = lineDirection.X / internalPiece.mLength;
		internalPiece.mLineDirection.Y = lineDirection.Y / internalPiece.mLength;

		internalPiece.mNormal.X = -internalPiece.mLineDirection.Y;
		internalPiece.mNormal.Y = internalPiece.mLineDirection.X;

		mSkeletonLines[frame].push_back(internalPiece);
	}
}