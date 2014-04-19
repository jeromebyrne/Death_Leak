#ifndef EFFECTLIGHTTEXTURE_H
#define EFFECTLIGHTTEXTURE_H

#include "effectbasic.h"

class EffectLightTexture : public EffectBasic
{
protected:
	// lighting vars
	ID3D10EffectVectorVariable* m_pLightDirVariable;
	ID3D10EffectVectorVariable* m_pLightColorVariable;

	// texture vars
	ID3D10EffectShaderResourceVariable* m_pDiffuseVariable;
	ID3D10ShaderResourceView* m_pTextureRV;

public:
	EffectLightTexture(void);
	virtual ~EffectLightTexture(void);
	virtual void Load(ID3D10Device* device);

	char* TextureFileName;

	void SetLightDirection(float * dir)
	{
		m_pLightDirVariable->SetFloatVector(dir);
	}
	void SetLightColor(float * col)
	{
		m_pLightColorVariable->SetFloatVector(col);
	}

	void SetTexture(ID3D10ShaderResourceView * tex);
};

#endif
