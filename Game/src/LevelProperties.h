#ifndef LEVELPROPERTIES_H
#define LEVELPROPERTIES_H

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
	void SetTargetOffset(Vector2 offset) { mTargetOffset = offset; }
	bool ShouldFollowX() const { return mFollowX; }
	bool ShouldFollowY() const { return mFollowY; }

private:

	Vector2 mCamBoundsTopLeft;
	Vector2 mCamBoundsBottomRight;
	float mCameraZoomInPercent;
	Vector2 mTargetOffset;
	Vector2 mOriginalTargetOffset;
	Vector2 mTargetLag;
	Vector2 mInitialCamPos;

	string mLevelMusic;
	float mMusicLength;
	float mMusicInitialDelay;
	float mMusicTimeBetween;
	bool mAllowWeather;

	bool mFollowX;
	bool mFollowY;
};

#endif
