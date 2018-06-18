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
		AnimationSkeletonFramePiece internalPiece = piece;

		mSkeletonLines[frame].push_back(internalPiece);
	}
}

bool AnimationSkeleton::HasCollidedOnFrame(unsigned int frame, 
											bool isHFlipped, 
											Vector2 & skeletonWorldPos, 
											Vector2 & startPoint, 
											Vector2 & endPoint,
											Vector2 & intersectPointOut)
{
	vector<AnimationSkeletonFramePiece> & vec = mSkeletonLines[frame];

	for (auto & piece : vec)
	{
		bool intersect = Intersect(isHFlipped,
									skeletonWorldPos,
									piece,
									startPoint,
									endPoint,
									intersectPointOut);

		if (intersect)
		{
			return true;
		}
	}

	return false;
}

bool AnimationSkeleton::Intersect(bool isHFlipped, 
									Vector2 & skeletonWorldPos, 
									AnimationSkeletonFramePiece & framePiece, 
									Vector2 & otherStart, 
									Vector2 & otherEnd,
									Vector2 & intersectPointOut)
{
	Vector2 pieceWorldStart(skeletonWorldPos.X + framePiece.mStartPos.X,
							skeletonWorldPos.Y + framePiece.mStartPos.Y);

	Vector2 pieceWorldEnd(skeletonWorldPos.X + framePiece.mEndPos.X,
							skeletonWorldPos.Y + framePiece.mEndPos.Y);

	if (isHFlipped)
	{
		pieceWorldStart.X = skeletonWorldPos.X - framePiece.mStartPos.X;
		pieceWorldEnd.X = skeletonWorldPos.X - framePiece.mEndPos.X;
	}
	
	// Calculate matrix determinants
	float det1 = (pieceWorldStart.X * pieceWorldEnd.Y) - (pieceWorldStart.Y * pieceWorldEnd.X);
	float det2 = (otherStart.X * otherEnd.Y) - (otherStart.Y * otherEnd.X);
	float det3 = ((pieceWorldStart.X - pieceWorldEnd.X) * (otherStart.Y - otherEnd.Y)) - ((pieceWorldStart.Y - pieceWorldEnd.Y) * (otherStart.X - otherEnd.X));

	if (det3 >= -0.00000001f && det3 <= 0.00000001f) return false;

	// Otherwise calculate the point of intersection:
	intersectPointOut.X = (det1 * (otherStart.X - otherEnd.X)) - ((pieceWorldStart.X - pieceWorldEnd.X) * det2);
	intersectPointOut.X /= det3;
	intersectPointOut.Y = (det1 * (otherStart.Y - otherEnd.Y)) - ((pieceWorldStart.Y - pieceWorldEnd.Y) * det2);
	intersectPointOut.Y /= det3;

	// Make sure the point is along both lines: get it relative to the start point of both lines
	Vector2 r1 = intersectPointOut - pieceWorldStart;
	Vector2 r2 = intersectPointOut - otherStart;

	Vector2 otherLineDistance = otherEnd - otherStart;
	Vector2 otherLineDirection = otherLineDistance;
	otherLineDirection.Normalise(); // TODO: optimisation opportunity

	// Do a dot product with both line directions to see if it is past the end of either of the two lines:
	float dot1 = r1.Dot(framePiece.mLineDirection);
	float dot2 = r2.Dot(otherLineDirection);

	// If either dot is negative then the point is past the beginning of one of the lines:
	if (dot1 < 0 || dot2 < 0)
	{
		return false;
	}

	// If either dot exceeds the length of the line then point is past the end of the line:
	if (dot1 > framePiece.mLength)
	{
		return false;
	}

	if (dot2 > otherLineDistance.Length()) // TODO: optimisation opportunity
	{
		return false;
	}

	return true;
}

bool AnimationSkeleton::HasBonesForFrame(unsigned int frame) 
{
	return mSkeletonLines[frame].size();
}

void AnimationSkeleton::ScaleBones(float scaleValue)
{
	for (auto & kvp : mSkeletonLines)
	{
		for (auto & piece : kvp.second)
		{
			piece.mStartPos = piece.mStartPos * scaleValue;
			piece.mEndPos = piece.mEndPos * scaleValue;

			Vector2 lineDirection = piece.mEndPos - piece.mStartPos;
			piece.mLength = lineDirection.Length();

			if (piece.mLength <= 0)
			{
				GAME_ASSERT(false);
				continue;
			}

			piece.mLineDirection.X = lineDirection.X / piece.mLength;
			piece.mLineDirection.Y = lineDirection.Y / piece.mLength;

			piece.mNormal.X = -piece.mLineDirection.Y;
			piece.mNormal.Y = piece.mLineDirection.X;
		}
	}
}