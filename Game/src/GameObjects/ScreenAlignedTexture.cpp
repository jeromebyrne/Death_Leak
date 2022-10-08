#include "precompiled.h"
#include "ScreenAlignedTexture.h"
#include "graphics.h"

ScreenAlignedTexture::ScreenAlignedTexture(void):
m_dimensions(200,200),
VertexBuffer(NULL),
IndexBuffer(NULL)
{
}

ScreenAlignedTexture::~ScreenAlignedTexture(void)
{
}


void ScreenAlignedTexture::Initialise()
{
	D3DXMatrixIdentity( &m_world );
	D3DXMatrixIdentity( &m_view );
	// create orthographic projection
	int bb_width = Graphics::GetInstance()->BackBufferWidth();
	int bb_height = Graphics::GetInstance()->BackBufferHeight();
	D3DXMatrixOrthoLH(&m_projection, (float)bb_width, (float)bb_height, 0.1f, 100000.0f);

	D3DXVECTOR2 tex1 = D3DXVECTOR2(0,1);
	D3DXVECTOR2 tex2 = D3DXVECTOR2(1,1);
	D3DXVECTOR2 tex3 = D3DXVECTOR2(1,0);
	D3DXVECTOR2 tex4 = D3DXVECTOR2(0,0);

	VertexPositionTextureNormal vertices[]=
    {
		{ D3DXVECTOR3( -m_dimensions.X/2, -m_dimensions.Y/2, 1 ), tex1, D3DXVECTOR3(0,1,0)}, // 0
        { D3DXVECTOR3( m_dimensions.X/2, -m_dimensions.Y/2, 1 ), tex2, D3DXVECTOR3(0,1,0)}, // 1
        { D3DXVECTOR3( m_dimensions.X/2, m_dimensions.Y/2, 1), tex3, D3DXVECTOR3(0,1,0)},// 2
        { D3DXVECTOR3( -m_dimensions.X/2, m_dimensions.Y/2, 1 ), tex4, D3DXVECTOR3(0,1,0)},// 3
    };
	
	for(int i = 0; i < 4; i++)
	{
		m_vertices[i] = vertices[i];
	}

	DWORD indices[] =
    {
        2,1,3,0
    };

	for(int i = 0; i < 4; i++)
	{
		m_indices[i] = indices[i];
	}
	
	ID3D10Device * device = Graphics::GetInstance()->Device();

	SetVertexBuffer(device, sizeof(m_vertices[0]) * 4, m_vertices);
	SetIndexBuffer(device, sizeof(m_indices[0]) * 4, m_indices);
}

void ScreenAlignedTexture::Draw()
{
	ID3D10Device * device = Graphics::GetInstance()->Device();

	m_effect->SetWorldViewProjection(m_world, m_view, m_projection);

	device->IASetInputLayout(m_effect->InputLayout);

	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &VertexBuffer, &stride, &offset);

	//// set the index buffer
	device->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effect->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0 , 0);
	}
}

void ScreenAlignedTexture::SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[])
{
	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    
	if(VertexBuffer)
	{
		VertexBuffer->Release();
	}
	
	device->CreateBuffer( &bd, &InitData, &VertexBuffer );
}

void ScreenAlignedTexture::SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[])
{	
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;        
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices;

	device->CreateBuffer( &bd, &InitData, &IndexBuffer );
}

void ScreenAlignedTexture::Release()
{
	if(VertexBuffer) 
	{
		VertexBuffer->Release();
		VertexBuffer = 0;
	}
	if(IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = 0;
	}
}
