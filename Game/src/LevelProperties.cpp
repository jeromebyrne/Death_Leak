#include "precompiled.h"
#include "LevelProperties.h"

LevelProperties::LevelProperties(void) :
	mCameraZoomInPercent(1.0f),
	mMusicInitialDelay(0.0f),
	mMusicLength(0.0f),
	mMusicTimeBetween(0.0f)
{

}

void LevelProperties::XmlRead(TiXmlElement * element)
{
	mCamBoundsTopLeft.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "left");
	mCamBoundsBottomRight.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "right");
	mCamBoundsTopLeft.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "top");
	mCamBoundsBottomRight.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "bottom");

	mCameraZoomInPercent = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "zoom_in_percent");
	mTargetOffset.X = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "xOffset");
	mTargetOffset.Y = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "yOffset");
	mTargetLag.X = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "xLag");
	mTargetLag.Y = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "yLag");

	mLevelMusic = XmlUtilities::ReadAttributeAsString(element, "music", "file");
	mMusicInitialDelay = XmlUtilities::ReadAttributeAsFloat(element, "music", "initial_delay");
	mMusicLength = XmlUtilities::ReadAttributeAsFloat(element, "music", "length");
	mMusicTimeBetween = XmlUtilities::ReadAttributeAsFloat(element, "music", "time_between");

	Camera2D * cam2d = Camera2D::GetInstance();
	if (cam2d)
	{
		cam2d->SetBounds(mCamBoundsTopLeft.X, mCamBoundsBottomRight.X, mCamBoundsTopLeft.Y, mCamBoundsBottomRight.Y);
		cam2d->SetZoomInLevel(mCameraZoomInPercent);
		cam2d->SetTargetOffset(mTargetOffset);
		cam2d->SetTargetLag(mTargetLag);
	}
}

void LevelProperties::XmlWrite(TiXmlElement * element)
{
	TiXmlElement * camProperties = new TiXmlElement("camera_properties");
	camProperties->SetDoubleAttribute("zoom_in_percent", mCameraZoomInPercent);
	camProperties->SetDoubleAttribute("xOffset", mTargetOffset.X);
	camProperties->SetDoubleAttribute("yOffset", mTargetOffset.Y);
	camProperties->SetDoubleAttribute("xLag", mTargetLag.X);
	camProperties->SetDoubleAttribute("yLag", mTargetLag.Y);
	element->LinkEndChild(camProperties);

	TiXmlElement * camBounds = new TiXmlElement("level_bounds");
	camBounds->SetDoubleAttribute("left", mCamBoundsTopLeft.X);
	camBounds->SetDoubleAttribute("right", mCamBoundsBottomRight.X);
	camBounds->SetDoubleAttribute("top", mCamBoundsTopLeft.Y);
	camBounds->SetDoubleAttribute("bottom", mCamBoundsBottomRight.Y);
	element->LinkEndChild(camBounds);

	mMusicInitialDelay = XmlUtilities::ReadAttributeAsFloat(element, "music", "initial_delay");
	mMusicLength = XmlUtilities::ReadAttributeAsFloat(element, "music", "length");
	mMusicTimeBetween = XmlUtilities::ReadAttributeAsFloat(element, "music", "time_between");

	TiXmlElement * music = new TiXmlElement("music");
	music->SetAttribute("file", mLevelMusic.c_str());
	music->SetDoubleAttribute("initial_delay", mMusicInitialDelay);
	music->SetDoubleAttribute("length", mMusicLength);
	music->SetDoubleAttribute("time_between", mMusicTimeBetween);
	element->LinkEndChild(music);

}

