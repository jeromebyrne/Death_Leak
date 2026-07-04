#ifndef LEVELPROPERTIES_H
#define LEVELPROPERTIES_H

#include "Vector2.h"
#include "tinyxml.h"

#include <string>

class LevelProperties
{
public:
	LevelProperties(void);
	~LevelProperties(void) {}

	void XmlRead(TiXmlElement * element);

	void XmlWrite(TiXmlElement * root);

	const char * GetLevelMusic() const { return mLevelMusic.c_str(); }
	float GetMusicLength() const { return mMusicLength; }
	float GetMusicInitialDelay() const { return mMusicInitialDelay; }
	float GetMusicTimeBetween() const  { return mMusicTimeBetween; }
	Vector2 GetTargetOffset() const { return mTargetOffset; }
	Vector2 GetOriginalTargetOffset() const { return mOriginalTargetOffset; }
	Vector2 GetTargetLag() const { return mTargetLag; }
	void SetTargetOffset(Vector2 offset) { mTargetOffset = offset; }
	bool ShouldFollowX() const { return mFollowX; }
	bool ShouldFollowY() const { return mFollowY; }
	bool IsAnimationPreview() const { return mIsAnimationPreview; }
	float GetZoomInPercent() const { return mCameraZoomInPercent; }
	Vector2 GetInitialCamPos() const { return mInitialCamPos; }
	Vector2 GetCamBoundsTopLeft() const { return mCamBoundsTopLeft; }
	Vector2 GetCamBoundsBottomRight() const { return mCamBoundsBottomRight; }

private:

	Vector2 mCamBoundsTopLeft;
	Vector2 mCamBoundsBottomRight;
	float mCameraZoomInPercent;
	Vector2 mTargetOffset;
	Vector2 mOriginalTargetOffset;
	Vector2 mTargetLag;
	Vector2 mInitialCamPos;
	std::string mLevelMusic;
	float mMusicLength;
	float mMusicInitialDelay;
	float mMusicTimeBetween;
	bool mAllowWeather;
	bool mFollowX;
	bool mFollowY;
	bool mIsAnimationPreview;
};

#endif
