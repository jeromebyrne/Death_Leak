#include "precompiled.h"
#include "TextObject.h"
#include "StringManager.h"

TextObject::TextObject(float x, float y, float z, float width, float height) :
	DrawableObject(x, y, z, width, height),
	mFont(nullptr),
	mFontSize(20.0f),
	mCachedWideString(nullptr)
{
}

TextObject::~TextObject(void)
{
}

void TextObject::Initialise()
{
	DrawableObject::Initialise();

	mLocalisedString = StringManager::GetInstance()->GetLocalisedString(mStringId.c_str());

	mCachedWideString = Utilities::ConvertCharStringToWcharString(mLocalisedString.c_str());

	// font setup
	{
		D3DX10_FONT_DESC fd;
		fd.Height = mFontSize;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, Utilities::ConvertCharStringToWcharString(mFontName.c_str()));

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mFont);
	}
}

void TextObject::Update(float delta)
{
	DrawableObject::Update(delta);
}

void TextObject::XmlRead(TiXmlElement * element)
{
	DrawableObject::XmlRead(element);

	mStringId = XmlUtilities::ReadAttributeAsString(element, "font_properties", "string_id");
	mFontName = XmlUtilities::ReadAttributeAsString(element, "font_properties", "name");
	mFontSize = XmlUtilities::ReadAttributeAsFloat(element, "font_properties", "size");

	mFontColor.r = XmlUtilities::ReadAttributeAsFloat(element, "font_color", "r");
	mFontColor.g = XmlUtilities::ReadAttributeAsFloat(element, "font_color", "g");
	mFontColor.b = XmlUtilities::ReadAttributeAsFloat(element, "font_color", "b");
	mFontColor.a = Alpha();
}

void TextObject::XmlWrite(TiXmlElement * element)
{
	DrawableObject::XmlWrite(element);

	TiXmlElement * fontProps = new TiXmlElement("font_properties");
	fontProps->SetAttribute("string_id", mStringId.c_str());
	fontProps->SetAttribute("name", mFontName.c_str());
	fontProps->SetDoubleAttribute("size", mFontSize);
	element->LinkEndChild(fontProps);

	TiXmlElement * fontColor = new TiXmlElement("font_color");
	fontColor->SetAttribute("r", mFontColor.r);
	fontColor->SetAttribute("g", mFontColor.g);
	fontColor->SetAttribute("b", mFontColor.b);
	element->LinkEndChild(fontColor);
}

void TextObject::Draw(ID3D10Device * device, Camera2D * camera)
{
	DrawableObject::Draw(device, camera);

	Vector2 worldPos = Vector2(m_position.X - (m_dimensions.X * 0.5f), m_position.Y);
	Vector2 screenPos = Utilities::WorldToScreen(worldPos);

	RECT bounds = { screenPos.X, screenPos.Y, screenPos.X + m_dimensions.X, screenPos.Y + m_dimensions.Y };

	mFontColor.a = Alpha();
	mFont->DrawText(0, mCachedWideString, -1, &bounds, DT_WORDBREAK, mFontColor);
}
