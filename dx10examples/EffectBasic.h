#ifndef EFFECTBASIC_H
#define EFFECTBASIC_H

#include "effectabstract.h"

class EffectBasic : public EffectAbstract
{
protected:
	// alpha
	ID3D10EffectScalarVariable * m_pAlphaVariable;
public:
	EffectBasic();
	virtual ~EffectBasic(void);
	virtual void Load(ID3D10Device* device);
	virtual void SetInputLayout(ID3D10EffectTechnique* technique, ID3D10Device* device, D3D10_INPUT_ELEMENT_DESC * layout, int numLayoutElements);
	
	void SetAlpha(float value)
	{
		if(value > 1)
		{
			value = 1;
		}
		else if(value < 0)
		{
			value = 0;
		}
		m_pAlphaVariable->SetFloat(value);
	}
};
#endif
