#ifndef EFFECTABSTRACT_H
#define EFFECTABSTRACT_H

#include "inputLayoutDescriptions.h"

class EffectAbstract
{
protected:
	// our dx effect variable
	ID3D10Effect* m_pEffect = nullptr;

	// every effect should have a world view projection dx variable
	ID3D10EffectMatrixVariable* m_pWorldVariable = nullptr;
    ID3D10EffectMatrixVariable* m_pViewVariable = nullptr;
    ID3D10EffectMatrixVariable* m_pProjectionVariable = nullptr;

public:

	// the input layout
	ID3D10InputLayout * InputLayout = nullptr;

	// the current effect technique
	ID3D10EffectTechnique * CurrentTechnique = nullptr;

	wchar_t* FileName = nullptr;

	void SetWorldViewProjection(float * pWorldData, float * pViewData, float * pProjData)
	{
		m_pWorldVariable->SetMatrix(pWorldData);
		m_pViewVariable->SetMatrix(pViewData);
		m_pProjectionVariable->SetMatrix(pProjData);
	}

	void SetView(float * pViewData)
	{
		m_pViewVariable->SetMatrix(pViewData);
	}

	void SetWorld(float * pWorldData)
	{
		m_pWorldVariable->SetMatrix(pWorldData);
	}
 
	EffectAbstract()
	{
		m_pEffect = 0;
		m_pWorldVariable = 0;
		m_pViewVariable = 0;
		m_pProjectionVariable = 0;
		InputLayout = 0;
		
		// set the default to basic shader
		FileName = L"ShaderFiles\\Basic.fx";
	}

	virtual ~EffectAbstract(void)
	{
		if( m_pEffect )
		{
			m_pEffect->Release();
			m_pEffect = nullptr;
		}

		if(InputLayout) 
		{
			InputLayout->Release();
			InputLayout = nullptr;
		}
	}
	
	// each effect should have different load code for different properties
	virtual void Load(ID3D10Device* device) = 0;
	virtual void SetInputLayout(ID3D10EffectTechnique* technique, ID3D10Device* device, D3D10_INPUT_ELEMENT_DESC * layout, int numLayoutElements) = 0;

	void SetCurrentTechnique(char* name)
	{
		CurrentTechnique = m_pEffect->GetTechniqueByName(name);
	}
};

#endif
