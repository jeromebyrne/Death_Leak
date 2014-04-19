#ifndef POSTEFFECTSEPIA_H
#define POSTEFFECTSEPIA_H

#include "effectlighttexture.h"

class EffectSepia  : public EffectLightTexture
{
private:

public:
	EffectSepia(void);
	~EffectSepia(void);
	virtual void Load(ID3D10Device * device);
};

#endif
