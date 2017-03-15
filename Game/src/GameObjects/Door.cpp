#include "precompiled.h"
#include "Door.h"

Door::Door() :
	Sprite()
{
}

Door::~Door()
{
}

void Door::Initialise()
{
	Sprite::Initialise();
}

void Door::Update(float delta)
{
	Sprite::Update(delta);
}
void Door::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

    /*
	mLerpStartPos.X = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "StartLerpX");
	mLerpStartPos.Y = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "StartLerpY");
	mLerpDistance.X = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "LerpDistanceX");
	mLerpDistance.Y = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "LerpDistanceY");
	mLerpTime = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "LerpTime");
	mFadeInTime = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "FadeInTime");
	mFadeOutTime = XmlUtilities::ReadAttributeAsFloat(element, "scroll_properties", "FadeOutTime");
     */
}

void Door::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);
    
    /*
	TiXmlElement * scrollPropsElem = new TiXmlElement("scroll_properties");
	scrollPropsElem->SetDoubleAttribute("StartLerpX", mLerpStartPos.X);
	scrollPropsElem->SetDoubleAttribute("StartLerpY", mLerpStartPos.Y);
	scrollPropsElem->SetDoubleAttribute("LerpDistanceX", mLerpDistance.X);
	scrollPropsElem->SetDoubleAttribute("LerpDistanceY", mLerpDistance.Y);
	scrollPropsElem->SetDoubleAttribute("LerpTime", mLerpTime);
	scrollPropsElem->SetDoubleAttribute("FadeInTime", mFadeInTime);
	scrollPropsElem->SetDoubleAttribute("FadeOutTime", mFadeOutTime);

	element->LinkEndChild(scrollPropsElem);
     */
}
