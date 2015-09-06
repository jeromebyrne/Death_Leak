#include "precompiled.h"
#include "UIMeter.h"

static const float kBarIncreaseTime = 0.25f; // time it takes to increase the bar from current progress to target progress

UIMeter::UIMeter(void): 
	UISprite(),
	mMeterLength(500),
	mProgress(1.0f),
	mCurrentState(kNormal),
	mTargetProgress(0),
	mProgressBeforeMovement(0),
	mTimeBeforeMovement(0.0f),
	mScaleX(1.0f),
	mScaleY(1.0f),
	mBarNativeHeight(0)
{
}

UIMeter::~UIMeter(void)
{
}

void UIMeter::Release()
{
	UISprite::Release();
}

void UIMeter::LoadContent(ID3D10Device * graphicsdevice)
{
	UISprite::LoadContent(graphicsdevice);

	m_meterEdgeTexture = TextureManager::Instance()->LoadTexture_ui(m_meterEdgeFilename.c_str());
	m_meterMiddleTexture = TextureManager::Instance()->LoadTexture_ui(m_meterMiddleFilename.c_str());
	m_meterBarMiddleTexture = TextureManager::Instance()->LoadTexture_ui(m_meterBarMiddleFilename.c_str());
	m_meterBarMiddleBackTexture = TextureManager::Instance()->LoadTexture_ui("Media\\UI\\meter_bar_middle_back.png");
}

void UIMeter::XmlRead(TiXmlElement * element)
{
	UISprite::XmlRead(element);
	
	m_meterBarMiddleFilename = XmlUtilities::ReadAttributeAsString(element, "", "meter_bar_middle");
	m_meterEdgeFilename = XmlUtilities::ReadAttributeAsString(element, "", "meter_edge");
	m_meterMiddleFilename = XmlUtilities::ReadAttributeAsString(element, "", "meter_middle");
}

void UIMeter::Draw(ID3D10Device * graphicsdevice)
{
	m_dimensions.Y = mBarNativeHeight * mScaleY;

	// unbind the vertex buffer before deleting it in Reset();
	UINT null = 0;
	ID3D10Buffer * nullB = nullptr;
	graphicsdevice->IASetVertexBuffers (0, 1, &nullB, &null, &null);

	if (mProgressBeforeMovement > mProgress && m_name.find("xp_meter") == std::string::npos)
	{
		// draw the white backing
		m_bottomLeft.X = mInitialPosition.X;
		m_bottomLeft.Y = mInitialPosition.Y;
		m_bottomLeft.X += (mMeterEdgeWidth * 0.9f) * mScaleX;
		m_horizontalFlip = false;
		m_dimensions.X = ((mMeterLength - (mMeterEdgeWidth * 0.8)) * mProgressBeforeMovement) * mScaleX;
		m_texture = m_meterBarMiddleBackTexture;

		Reset(); 
		UISprite::Draw(graphicsdevice);
	}

	// draw the bar
	m_bottomLeft.X = mInitialPosition.X;
	m_bottomLeft.Y = mInitialPosition.Y;
	m_bottomLeft.X += (mMeterEdgeWidth * 0.9f) * mScaleX;
	m_horizontalFlip = false;
	m_dimensions.X = ((mMeterLength - (mMeterEdgeWidth * 0.8)) * mProgress) * mScaleX;
	m_texture = m_meterBarMiddleTexture;

	Reset(); 
	UISprite::Draw(graphicsdevice);
	
	// draw meter left edge
	m_bottomLeft.X = mInitialPosition.X;
	m_bottomLeft.Y = mInitialPosition.Y;
	m_horizontalFlip = false;
	m_dimensions.X = mMeterEdgeWidth * mScaleX;
	m_texture = m_meterEdgeTexture;
	
	// unbind the vertex buffer before deleting it in Reset();
	graphicsdevice->IASetVertexBuffers (0, 1, &nullB, &null, &null);

	Reset(); 
	UISprite::Draw(graphicsdevice);

	// draw meter right edge
	m_bottomLeft.X = mInitialPosition.X;
	m_bottomLeft.Y = mInitialPosition.Y;
	m_horizontalFlip = true;
	m_bottomLeft.X += mMeterLength * mScaleX;
	m_dimensions.X = mMeterEdgeWidth * mScaleX;
	m_texture = m_meterEdgeTexture;

	// unbind the vertex buffer before deleting it in Reset();
	graphicsdevice->IASetVertexBuffers (0, 1, &nullB, &null, &null);

	Reset(); 
	UISprite::Draw(graphicsdevice);

	// draw meter middle
	m_bottomLeft.X = mInitialPosition.X;
	m_bottomLeft.Y = mInitialPosition.Y;
	m_horizontalFlip = false;
	m_bottomLeft.X += mMeterEdgeWidth * mScaleX;
	m_dimensions.X = (mMeterLength - mMeterEdgeWidth * 0.95) * mScaleX;
	m_texture = m_meterMiddleTexture;

	// unbind the vertex buffer before deleting it in Reset();
	graphicsdevice->IASetVertexBuffers (0, 1, &nullB, &null, &null);

	Reset(); 
	UISprite::Draw(graphicsdevice);

}

void UIMeter::Initialise()
{
	UISprite::Initialise();

	// get the height of the bar texture 
	ID3D10Texture2D * tex2d;
	m_meterEdgeTexture->GetResource( reinterpret_cast<ID3D10Resource**>(&tex2d));
	D3D10_TEXTURE2D_DESC desc;
	tex2d->GetDesc(&desc);
	m_dimensions.Y = desc.Height;
	mMeterEdgeWidth = desc.Width;
	mBarNativeHeight = desc.Height;
	mInitialPosition = m_bottomLeft;
}

void UIMeter::Scale(float x, float y)
{
	UIWidget::Scale(x,y);

	mScaleX = x;
	mScaleY = y;

	mMeterLength = m_dimensions.X;
}

void UIMeter::Reset()
{
	UISprite::Reset();
}

void UIMeter::SetProgress(float targetProgress)
{
	if (targetProgress != mProgress)
	{
		if (mCurrentState != kBarMoving)
		{
			mProgressBeforeMovement = mProgress;
			mTimeBeforeMovement = Timing::Instance()->GetTotalTimeSeconds();
		}
		mTargetProgress = targetProgress;
		mCurrentState = kBarMoving;
	}
}

void UIMeter::Update()
{
	switch (mCurrentState)
	{
		case kNormal:
			{
				if (mProgressBeforeMovement > mProgress)
				{
					mProgressBeforeMovement -= 0.005f;
				}
				else
				{
					mProgressBeforeMovement = mProgress;
				}
				break;
			}
		case kBarMoving:
			{
				float progress_diff = mTargetProgress - mProgressBeforeMovement;

				// hack
				if (progress_diff < 0 && m_name.find("xp_meter") != std::string::npos)
				{
					mProgress = mTargetProgress;
					mCurrentState = kNormal;
					return;
				}

				// get the percentage of time that has passed 
				float time_spent = Timing::Instance()->GetTotalTimeSeconds() - mTimeBeforeMovement;
				
				if (time_spent > 0)
				{
					// this is the percentage of the total that needs to be added/decreased from the bar
					float total_percent_time =  (float)(min(time_spent, kBarIncreaseTime)) / (float)(kBarIncreaseTime);

					mProgress = mProgressBeforeMovement + (progress_diff * total_percent_time);

					if (total_percent_time >= 1)
					{
						mProgress = mTargetProgress;
						mCurrentState = kNormal;
					}
				}

				break;
			}
	};
}