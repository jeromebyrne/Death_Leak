#ifndef SCROLLINGSPRITE_H
#define SCROLLINGSPRITE_H

#include "sprite.h"

class ScrollingSprite : public Sprite
{
public:

	ScrollingSprite();
	virtual ~ScrollingSprite(void);

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

};

#endif
