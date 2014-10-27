#ifndef LEVELPROPERTIES_H
#define LEVELPROPERTIES_H

class LevelProperties
{
public:
	LevelProperties(void);
	~LevelProperties(void) {}

	void XmlRead(TiXmlElement * element);

	void XmlWrite(TiXmlElement * root);

private:

	Vector2 mCamBoundsTopLeft;
	Vector2 mCamBoundsBottomRight;
	float mCameraZoomInPercent;
	Vector2 mTargetOffset;
	Vector2 mTargetLag;
};

#endif
