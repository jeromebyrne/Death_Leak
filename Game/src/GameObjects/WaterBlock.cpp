#include "precompiled.h"
#include "WaterBlock.h"

WaterBlock::WaterBlock(void)
{
}

WaterBlock::~WaterBlock(void)
{
}

void WaterBlock::OnCollision(SolidMovingSprite * object)
{
	object->SetIsInWater(true);
	/*Platform::OnCollision(object);
	
	if (mPathingType == kPathWhenTriggered &&
		(mCurrentPathState == kNotPathing || mCurrentPathState == kReturningToStart) && 
		dynamic_cast<Player*>(object))
	{
		if(object->Bottom() > Y () && object->X() > Left() && object->X() < Right() && object->VelocityY() <= 0.0)
		{
			mClosestPointToNextTarget = Vector3(kLargestInt, kLargestInt, 0);

			if (mPathForward)
			{
				if (mCurrentPathState == kReturningToStart)
				{
					if (mCurrentPathIndex < (mPathPoints.size()-1))
					{
						++mCurrentPathIndex;
					}
					else
					{
						mCurrentPathIndex = 0;
					}
				}
				mCurrentPathState = kLoopingForward;
			}
			else
			{
				if (mCurrentPathState == kReturningToStart)
				{
					if (mCurrentPathIndex > 0)
					{
						--mCurrentPathIndex;
					}
				}
				mCurrentPathState = kLoopingBackward;
			}
		}
	}*/
}
