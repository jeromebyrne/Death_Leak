#ifndef EFFECTMONOCHROME_H
#define EFFECTMONOCHROME_H

#include "effectlighttexture.h"

class EffectMonochrome : public EffectLightTexture
{
public:
	EffectMonochrome(void);
	~EffectMonochrome(void);

	virtual void Load(ID3D10Device * device);
};

#endif