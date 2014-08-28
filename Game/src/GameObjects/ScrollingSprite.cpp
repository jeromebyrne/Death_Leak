#include "precompiled.h"
#include "ScrollingSprite.h"
#include "Game.h"

ScrollingSprite::ScrollingSprite() : 
	Sprite(),
	mLerpTime(0.0f),
	mCurrentLerpTime(0.0f),
	mFadeInTime(0.0f),
	mFadeOutTime(0.0f)
{
}

ScrollingSprite::~ScrollingSprite()
{
}

void ScrollingSprite::Initialise()
{
	Sprite::Initialise();
}

void ScrollingSprite::Update(float delta)
{
	Sprite::Update(delta);

	mCurrentLerpTime += delta;
	if (mCurrentLerpTime >= mLerpTime)
	{
		mCurrentLerpTime = 0.0f;
		return; // skip a frame
	}

#if _DEBUG
	if (Game::GetIsLevelEditMode())
	{
		return;
	}
#endif

	GAME_ASSERT(mLerpTime > 0.0f);
	float percent = mCurrentLerpTime / mLerpTime;

	m_position.X = mLerpStartPos.X + (mLerpDistance.X * percent);
	m_position.Y = mLerpStartPos.Y + (mLerpDistance.Y * percent);

	if (mCurrentLerpTime < mFadeInTime)
	{
		m_alpha = mOriginalAlpha * (mCurrentLerpTime / mFadeInTime);
	}
	else if (mCurrentLerpTime > mLerpTime - mFadeOutTime)
	{
		m_alpha = m_alpha = mOriginalAlpha * ((mLerpTime - mCurrentLerpTime) / mFadeOutTime);
	}
	else
	{
		m_alpha = mOriginalAlpha;
	}
}
void ScrollingSprite::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	mLerpStartPos.X = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "StartLerpX");
	mLerpStartPos.Y = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "StartLerpY");
	mLerpDistance.X = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "LerpDistanceX");
	mLerpDistance.Y = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "LerpDistanceY");
	mLerpTime = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "LerpTime");
	mFadeInTime = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "FadeInTime");
	mFadeOutTime = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "FadeOutTime");
}

void ScrollingSprite::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * scrollPropsElem = new TiXmlElement("scroll_properties");
	scrollPropsElem->SetDoubleAttribute("StartLerpX", mLerpStartPos.X);
	scrollPropsElem->SetDoubleAttribute("StartLerpY", mLerpStartPos.Y);
	scrollPropsElem->SetDoubleAttribute("LerpDistanceX", mLerpDistance.X);
	scrollPropsElem->SetDoubleAttribute("LerpDistanceY", mLerpDistance.Y);
	scrollPropsElem->SetDoubleAttribute("LerpTime", mLerpTime);
	scrollPropsElem->SetDoubleAttribute("FadeInTime", mFadeInTime);
	scrollPropsElem->SetDoubleAttribute("FadeOutTime", mFadeOutTime);

	element->LinkEndChild(scrollPropsElem);
}
