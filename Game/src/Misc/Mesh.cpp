#include "precompiled.h"
#include "Mesh.h"

Mesh::Mesh(void)
{
	// Initialize the world matrix
    D3DXMatrixIdentity( &m_world );
	D3DXMatrixScaling(&m_world, 0.1,0.1,0.1);
}

Mesh::~Mesh(void)
{
}

void Mesh::Load(wchar_t* fileName, ID3D10Device * graphicsDevice)
{
	m_mesh.Create( graphicsDevice, (LPCTSTR)fileName, true );
}

void Mesh::Update(D3DXMATRIX globalWorld)
{
	D3DXMatrixMultiply( &m_world, &m_world, &globalWorld); // take the global world into account
}

void Mesh::Draw(ID3D10Device * graphicsDevice, ID3D10EffectTechnique* technique, ID3D10EffectShaderResourceVariable* shaderResourceVar)
{
	UINT Strides[1];
    UINT Offsets[1];
    ID3D10Buffer* pVB[1];
    pVB[0] = m_mesh.GetVB10( 0, 0 );
    Strides[0] = ( UINT )m_mesh.GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    graphicsDevice->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    graphicsDevice->IASetIndexBuffer( m_mesh.GetIB10( 0 ), m_mesh.GetIBFormat10( 0 ), 0 );

    D3D10_TECHNIQUE_DESC techDesc;
    technique->GetDesc( &techDesc );
    SDKMESH_SUBSET* pSubset = NULL;
    ID3D10ShaderResourceView* pDiffuseRV = NULL;
    D3D10_PRIMITIVE_TOPOLOGY PrimType;

    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        for( UINT subset = 0; subset < m_mesh.GetNumSubsets( 0 ); ++subset )
        {
            pSubset = m_mesh.GetSubset( 0, subset );

            PrimType = m_mesh.GetPrimitiveType10( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
            graphicsDevice->IASetPrimitiveTopology( PrimType );

            pDiffuseRV = m_mesh.GetMaterial( pSubset->MaterialID )->pDiffuseRV10;
            shaderResourceVar->SetResource( pDiffuseRV );

            technique->GetPassByIndex( p )->Apply( 0 );
            graphicsDevice->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
        }
    }

	//the mesh class also had a render method that allows rendering the mesh with the most common options
    //g_Mesh.Render( pd3dDevice, g_pTechnique, g_ptxDiffuseVariable );
}
