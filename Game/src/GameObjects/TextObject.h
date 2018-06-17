#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include "drawableobject.h"

class TextObject : public DrawableObject
{
public:

	TextObject(float x = 0, float y = 0, DepthLayer depthLayer = kPlayer, float width = 1, float height = 1);
	virtual ~TextObject();

	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void Initialise() override;

	void SetStringKey(const char * key);
	void SetFont(const char * fontname);
	void SetFontSize(float size);
	void SetFontColor(float r, float g, float b);

	void SetNoClip(bool centred) { mNoClip = centred; }

private:

	ID3DX10Font* mFont;
	D3DXCOLOR mFontColor;
	std::string mFontName;
	float mFontSize;
	std::string mStringId;
	std::string mLocalisedString;

	wchar_t * mCachedWideString;

	bool mHasShown;

	float mTimeToShow;

	bool mNoClip;
};

#endif

