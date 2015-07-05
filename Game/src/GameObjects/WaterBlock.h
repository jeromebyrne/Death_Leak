#ifndef WATERBLOCK_H
#define WATERBLOCK_H

#include "solidmovingsprite.h"

class WaterBlock : public SolidMovingSprite
{
public:
	WaterBlock(void);
	virtual ~WaterBlock(void);

	virtual bool OnCollision(SolidMovingSprite * object) override;

	bool GetIsDeepWater() const { return mIsDeepWater; }

private:

	void XmlRead(TiXmlElement * element) override;

	void XmlWrite(TiXmlElement * element) override;

	bool mIsDeepWater;
};

#endif

