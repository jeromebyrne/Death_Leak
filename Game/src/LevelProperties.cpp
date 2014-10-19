#include "precompiled.h"
#include "LevelProperties.h"

void LevelProperties::XmlRead(TiXmlElement * element)
{
	mCamBoundsTopLeft.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "left");
	mCamBoundsBottomRight.X = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "right");
	mCamBoundsTopLeft.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "top");
	mCamBoundsBottomRight.Y = XmlUtilities::ReadAttributeAsFloat(element, "level_bounds", "bottom");

	Camera2D * cam2d = Camera2D::GetInstance();
	if (cam2d)
	{
		cam2d->SetBounds(mCamBoundsTopLeft.X, mCamBoundsBottomRight.X, mCamBoundsTopLeft.Y, mCamBoundsBottomRight.Y);
	}
}

void LevelProperties::XmlWrite(TiXmlElement * element)
{
	// position
	TiXmlElement * camBounds = new TiXmlElement("level_bounds");
	camBounds->SetDoubleAttribute("left", mCamBoundsTopLeft.X);
	camBounds->SetDoubleAttribute("right", mCamBoundsBottomRight.X);
	camBounds->SetDoubleAttribute("top", mCamBoundsTopLeft.Y);
	camBounds->SetDoubleAttribute("bottom", mCamBoundsBottomRight.Y);
	element->LinkEndChild(camBounds);
}

