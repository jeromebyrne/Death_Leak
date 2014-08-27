#include "precompiled.h"
#include "ScrollingSprite.h"

ScrollingSprite::ScrollingSprite() : 
	Sprite()
{
}

ScrollingSprite::~ScrollingSprite()
{
}

void ScrollingSprite::Initialise()
{
	Sprite::Initialise();
}

void ScrollingSprite::Update(float delta)
{
	Sprite::Update(delta);
}
void ScrollingSprite::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);
}

void ScrollingSprite::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);
}
