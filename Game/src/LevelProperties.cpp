#include "precompiled.h"
#include "LevelProperties.h"

LevelProperties::LevelProperties(void) :
	mCameraZoomInPercent(1.0f)
{

}

void LevelProperties::XmlRead(TiXmlElement * element)
{
	mCamBoundsTopLeft.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "left");
	mCamBoundsBottomRight.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "right");
	mCamBoundsTopLeft.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "top");
	mCamBoundsBottomRight.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "bottom");
	mCameraZoomInPercent = XmlUtilities::ReadAttributeAsFloat(element, "zoom_in_percent", "value");

	Camera2D * cam2d = Camera2D::GetInstance();
	if (cam2d)
	{
		cam2d->SetBounds(mCamBoundsTopLeft.X, mCamBoundsBottomRight.X, mCamBoundsTopLeft.Y, mCamBoundsBottomRight.Y);
		cam2d->SetZoomInLevel(mCameraZoomInPercent);
	}
}

void LevelProperties::XmlWrite(TiXmlElement * element)
{
	TiXmlElement * camZoom = new TiXmlElement("zoom_in_percent");
	camZoom->SetDoubleAttribute("value", mCameraZoomInPercent);
	element->LinkEndChild(camZoom);

	TiXmlElement * camBounds = new TiXmlElement("level_bounds");
	camBounds->SetDoubleAttribute("left", mCamBoundsTopLeft.X);
	camBounds->SetDoubleAttribute("right", mCamBoundsBottomRight.X);
	camBounds->SetDoubleAttribute("top", mCamBoundsTopLeft.Y);
	camBounds->SetDoubleAttribute("bottom", mCamBoundsBottomRight.Y);
	element->LinkEndChild(camBounds);
}

