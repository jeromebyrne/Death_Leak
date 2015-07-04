#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include "drawableobject.h"

class TextObject : public DrawableObject
{
public:
	TextObject(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1);
	virtual ~TextObject();

	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void Initialise() override;

private:

	ID3DX10Font* mFont;
	D3DXCOLOR mFontColor;
	std::string mFontName;
	float mFontSize;
	std::string mStringId;
	std::string mLocalisedString;

	wchar_t * mCachedWideString;
};

#endif

