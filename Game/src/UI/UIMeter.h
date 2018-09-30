#ifndef UIMETER_H
#define UIMETER_H

#include "uisprite.h"

class UIMeter : public UISprite
{

public:
	UIMeter(void);
	virtual ~UIMeter(void);

	virtual void Release();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
	virtual void Draw(ID3D10Device * graphicsdevice);
	virtual void Initialise();

	virtual void Reset();
	virtual void Update();

	virtual void SetProgress(float percent);

	virtual void Scale(float x, float y);

	void SetMeterLength(float x);

	float GetMeterLength() const { return mMeterLength; }

	void SetDrawMeter(bool value) { mDrawMeter = value; }

private:

	enum State
	{
		kNormal,
		kBarMoving
	};

	ID3D10ShaderResourceView * m_meterEdgeTexture;
	ID3D10ShaderResourceView * m_meterMiddleTexture;
	ID3D10ShaderResourceView * m_meterBarMiddleTexture;
	ID3D10ShaderResourceView * m_meterBarMiddleBackTexture;
	string m_meterEdgeFilename;
	string m_meterMiddleFilename;
	string m_meterBarMiddleFilename;

	float mMeterLength;
	float mMeterEdgeWidth; // meter edge shouldnt scale so keep note of it's width
	float mProgress; // float between 0 and 1
	
	// keep hold of the initial position for drawing
	Vector2 mInitialPosition;

	State mCurrentState;

	// the progress that the bar wants to move to
	float mTargetProgress;
	
	float mProgressBeforeMovement;
	float mTimeBeforeMovement;

	float mScaleX;
	float mScaleY;

	float mBarNativeHeight;

	bool mDrawMeter = true;
};

#endif
