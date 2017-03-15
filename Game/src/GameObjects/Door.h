#ifndef DOOR_H
#define DOOR_H

#include "sprite.h"

class Door : public Sprite
{
public:

	Door();
	virtual ~Door(void);

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

};

#endif
