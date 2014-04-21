#include "precompiled.h"
#include "EffectBasic.h"

EffectBasic::EffectBasic() 
: EffectAbstract()
{
	
}

EffectBasic::~EffectBasic(void)
{
}

void EffectBasic::Load(ID3D10Device* device)
{
	HRESULT hr = S_OK;

	// Create the effect
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif
    hr = D3DX10CreateEffectFromFile(FileName, NULL, NULL, "fx_4_0", dwShaderFlags, 0,
		device, NULL, NULL, &m_pEffect, NULL, NULL );

    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    TEXT("The FX file cannot be loaded.  Please run this executable from the directory that contains the FX file."), TEXT("Error"), MB_OK );
    }
	
	// now get our world, view and proj from the effect file
	m_pWorldVariable = m_pEffect->GetVariableByName( "World" )->AsMatrix();
    m_pViewVariable = m_pEffect->GetVariableByName( "View" )->AsMatrix();
    m_pProjectionVariable = m_pEffect->GetVariableByName( "Projection" )->AsMatrix();

	// set the current technique to the default "render"
	CurrentTechnique = m_pEffect->GetTechniqueByName("Render");

	// get the alpha variable
	m_pAlphaVariable = m_pEffect->GetVariableByName( "alpha" )->AsScalar();

	// alpha defaults to 1
	SetAlpha(1.0f);
}

void EffectBasic::SetInputLayout(ID3D10EffectTechnique* technique, ID3D10Device* device, D3D10_INPUT_ELEMENT_DESC * layout, int numLayoutElements)
{
    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    technique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	device->CreateInputLayout( layout, numLayoutElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &InputLayout );
}
