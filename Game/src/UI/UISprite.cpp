#include "precompiled.h"
#include "UISprite.h"
#include "UIManager.h"
#include "EffectLightTextureVertexWobble.h"

UISprite::UISprite(void) : UIWidget(),
	m_horizontalFlip(false),
	m_verticalFlip(false),
	m_applyChange(false),
	VertexBuffer(0),
	IndexBuffer(0),
	mUseStandardEffect(false)
{
}

UISprite::~UISprite(void)
{
}

void UISprite::XmlRead(TiXmlElement * element)
{
	UIWidget::XmlRead(element);

	m_textureFilename = XmlUtilities::ReadAttributeAsString(element, "", "image");
	m_horizontalFlip = XmlUtilities::ReadAttributeAsBool(element, "", "hflip");
	m_verticalFlip = XmlUtilities::ReadAttributeAsBool(element, "", "vflip");
	m_alpha = XmlUtilities::ReadAttributeAsFloat(element, "", "alpha");

	bool attribExists = XmlUtilities::AttributeExists(element, "", "use_standard_effect");

	if (attribExists)
	{
		mUseStandardEffect = XmlUtilities::ReadAttributeAsBool(element, "", "use_standard_effect");
	}
}

void UISprite::SetImage(const char * filename)
{
	m_textureFilename = filename;
}

void UISprite::Release()
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

	UIWidget::Release();
}

void UISprite::Reset()
{
	// need to reset vertices and vertex buffer
	D3DXVECTOR2 tex1 = D3DXVECTOR2(0,0);
	D3DXVECTOR2 tex2 = D3DXVECTOR2(0,0);
	D3DXVECTOR2 tex3 = D3DXVECTOR2(0,0);
	D3DXVECTOR2 tex4 = D3DXVECTOR2(0,0);

	if(m_horizontalFlip)
	{
		tex1.x = 1;
		tex2.x = 0;
		tex3.x = 0;
		tex4.x = 1;

	}
	else
	{
		tex1.x = 0;
		tex2.x = 1;
		tex3.x = 1;
		tex4.x = 0;
	}

	if(m_verticalFlip)
	{
		tex1.y = 1;
		tex2.y = 1;
		tex3.y = 0;
		tex4.y = 0;
	}
	else
	{
		tex1.y = 0;
		tex2.y = 0;
		tex3.y = 1;
		tex4.y = 1;
	}

	VertexPositionTextureNormal vertices[]=
    {
		{ D3DXVECTOR3( m_bottomLeft.X, m_bottomLeft.Y, 2 ), tex1, D3DXVECTOR3(0,1,0)}, // 0
        { D3DXVECTOR3( m_bottomLeft.X + m_dimensions.X, m_bottomLeft.Y, 2 ), tex2, D3DXVECTOR3(0,1,0)}, // 1
        { D3DXVECTOR3( m_bottomLeft.X + m_dimensions.X, m_bottomLeft.Y + m_dimensions.Y, 2), tex3, D3DXVECTOR3(0,1,0)},// 2
        { D3DXVECTOR3( m_bottomLeft.X, m_bottomLeft.Y + m_dimensions.Y, 2 ), tex4, D3DXVECTOR3(0,1,0)},// 3
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

	SetVertexBuffer(Graphics::GetInstance()->Device(), sizeof(m_vertices[0]) * 4, m_vertices);
	//SetIndexBuffer(Graphics::GetInstance()->Device(), sizeof(m_indices[0]) * 4, m_indices);
}

void UISprite::LoadContent(ID3D10Device * graphicsdevice)
{
	UIWidget::LoadContent(graphicsdevice);

	// load the sprite texture (store in the ui texture cache)
	m_texture = TextureManager::Instance()->LoadTexture_ui((char*)m_textureFilename.c_str());
	
	RebuildBuffers();
}

void UISprite::RebuildBuffers()
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}

	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	D3DXVECTOR2 tex1;
	D3DXVECTOR2 tex2;
	D3DXVECTOR2 tex3;
	D3DXVECTOR2 tex4;

	if(m_horizontalFlip)
	{
		tex1.x = 1;
		tex2.x = 0;
		tex3.x = 0;
		tex4.x = 1;
	}
	else
	{
		tex1.x = 0;
		tex2.x = 1;
		tex3.x = 1;
		tex4.x = 0;
	}

	if(m_verticalFlip)
	{
		tex1.y = 1;
		tex2.y = 1;
		tex3.y = 0;
		tex4.y = 0;
	}
	else
	{
		tex1.y = 0;
		tex2.y = 0;
		tex3.y = 1;
		tex4.y = 1;
	}

	VertexPositionTextureNormal vertices[]=
    {
		{ D3DXVECTOR3( m_bottomLeft.X, m_bottomLeft.Y, 2 ), tex1, D3DXVECTOR3(0,1,0)}, // 0
        { D3DXVECTOR3( m_bottomLeft.X + m_dimensions.X, m_bottomLeft.Y, 2 ), tex2, D3DXVECTOR3(0,1,0)}, // 1
        { D3DXVECTOR3( m_bottomLeft.X + m_dimensions.X, m_bottomLeft.Y + m_dimensions.Y, 2), tex3, D3DXVECTOR3(0,1,0)},// 2
        { D3DXVECTOR3( m_bottomLeft.X, m_bottomLeft.Y + m_dimensions.Y, 2 ), tex4, D3DXVECTOR3(0,1,0)},// 3
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

	SetVertexBuffer(Graphics::GetInstance()->Device(), sizeof(m_vertices[0]) * 4, m_vertices);
	SetIndexBuffer(Graphics::GetInstance()->Device(), sizeof(m_indices[0]) * 4, m_indices);
}

void UISprite::SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[])
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

void UISprite::SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[])
{
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;        
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices;

	if(IndexBuffer)
	{
		IndexBuffer->Release();
	}

	device->CreateBuffer( &bd, &InitData, &IndexBuffer );
}

void UISprite::Initialise()
{
	// initialise world matrix for sprite
	D3DXMatrixIdentity( &m_world );
}

void UISprite::Draw(ID3D10Device * graphicsdevice)
{
	EffectLightTexture * effect = UIManager::Instance()->GetDefaultEffect();

	if (mUseStandardEffect)
	{
		effect = UIManager::Instance()->GetStandardEffect();
	}

	// set the texture
	effect->SetTexture(m_texture);

	// set the alpha value
	effect->SetAlpha(m_alpha);

	// set the input layout
	graphicsdevice->IASetInputLayout(effect->InputLayout);

	// set the vertex buffer data
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	graphicsdevice->IASetVertexBuffers(0,1,&VertexBuffer, &stride, &offset);

	// set the index buffer
	graphicsdevice->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	graphicsdevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	effect->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		effect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		graphicsdevice->DrawIndexed(4, 0 , 0);
	}
}
