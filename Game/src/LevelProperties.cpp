#include "precompiled.h"
#include "LevelProperties.h"
#include "WeatherManager.h"

LevelProperties::LevelProperties(void) :
	mCameraZoomInPercent(1.0f),
	mMusicInitialDelay(0.0f),
	mMusicLength(0.0f),
	mMusicTimeBetween(0.0f),
	mAllowWeather(true),
	mFollowX(true),
	mFollowY(true),
	mIsAnimationPreview(false)
{
}

void LevelProperties::XmlRead(TiXmlElement * element)
{
	mCamBoundsTopLeft.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "left");
	mCamBoundsBottomRight.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "right");
	mCamBoundsTopLeft.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "top");
	mCamBoundsBottomRight.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "bottom");

	mInitialCamPos.X = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "posx");
	mInitialCamPos.Y = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "posy");
	mCameraZoomInPercent = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "zoom_in_percent");
	mTargetOffset.X = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "xOffset");
	mTargetOffset.Y = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "yOffset");
	mTargetLag.X = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "xLag");
	mTargetLag.Y = XmlUtilities::ReadAttributeAsFloat(element, "camera_properties", "yLag");
	mFollowX = XmlUtilities::ReadAttributeAsBool(element, "camera_properties", "followx");
	mFollowY = XmlUtilities::ReadAttributeAsBool(element, "camera_properties", "followy");

	mIsAnimationPreview = XmlUtilities::ReadAttributeAsBool(element, "is_animation_preview", "value");

	mOriginalTargetOffset = mTargetOffset;

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
		cam2d->SetShouldFollowX(mFollowX);
		cam2d->SetShouldFollowY(mFollowY);
		cam2d->SetPositionX(mInitialCamPos.X);
		cam2d->SetPositionY(mInitialCamPos.Y);
	}

	mAllowWeather = XmlUtilities::ReadAttributeAsBool(element, "weather_properties", "allow_weather");
	WeatherManager::GetInstance()->SetAllowWeather(mAllowWeather);

	if (!mAllowWeather)
	{
		WeatherManager::GetInstance()->StopAllWeather();
	}
}

void LevelProperties::XmlWrite(TiXmlElement * element)
{
	TiXmlElement * camProperties = new TiXmlElement("camera_properties");
	camProperties->SetDoubleAttribute("posx", mInitialCamPos.X);
	camProperties->SetDoubleAttribute("posy", mInitialCamPos.Y);
	camProperties->SetDoubleAttribute("zoom_in_percent", mCameraZoomInPercent);
	camProperties->SetDoubleAttribute("xOffset", mTargetOffset.X);
	camProperties->SetDoubleAttribute("yOffset", mTargetOffset.Y);
	camProperties->SetDoubleAttribute("xLag", mTargetLag.X);
	camProperties->SetDoubleAttribute("yLag", mTargetLag.Y);
	camProperties->SetAttribute("followx", mFollowX ? "true" : "false");
	camProperties->SetAttribute("followy", mFollowY ? "true" : "false");
	element->LinkEndChild(camProperties);

	TiXmlElement * camBounds = new TiXmlElement("level_bounds");
	camBounds->SetDoubleAttribute("left", mCamBoundsTopLeft.X);
	camBounds->SetDoubleAttribute("right", mCamBoundsBottomRight.X);
	camBounds->SetDoubleAttribute("top", mCamBoundsTopLeft.Y);
	camBounds->SetDoubleAttribute("bottom", mCamBoundsBottomRight.Y);
	element->LinkEndChild(camBounds);

	TiXmlElement * music = new TiXmlElement("music");
	music->SetAttribute("file", mLevelMusic.c_str());
	music->SetDoubleAttribute("initial_delay", mMusicInitialDelay);
	music->SetDoubleAttribute("length", mMusicLength);
	music->SetDoubleAttribute("time_between", mMusicTimeBetween);
	element->LinkEndChild(music);

	TiXmlElement * weather = new TiXmlElement("weather_properties");
	weather->SetAttribute("allow_weather", mAllowWeather ? "true" : "false");
	element->LinkEndChild(weather);

	TiXmlElement * animationPreview = new TiXmlElement("is_animation_preview");
	animationPreview->SetAttribute("value", mIsAnimationPreview ? "true" : "false");
	element->LinkEndChild(animationPreview);
}

