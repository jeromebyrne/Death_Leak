#ifndef DRAW_UTILITIES
#define DRAW_UTILITIES

#include "gameobject.h"
#include "sprite.h"
#include "effectmanager.h"
#include "EffectBasic.h"
#include "EffectLightTexture.h"
#include "EffectLightTextureVertexWobble.h"
#include "EffectReflection.h"
#include "EffectParticleSpray.h"

// Should only really used for debug drawing, these functions are quick and dirty and not very efficient

class DrawUtilities
{
private:

public:
	DrawUtilities(void) {}
	~DrawUtilities(void) {}

	static void DrawCircle(Vector3 centrePoint, float radius)
	{
		// get the effect
		EffectLightTexture * effect = static_cast<EffectLightTexture*>(EffectManager::Instance()->GetEffect("effectlighttexture"));

		// reset world matrix
		D3DXMATRIX identity;
		D3DXMatrixIdentity( &identity );
		effect->SetWorld((float*)&identity);
		
		// get the texture
		ID3D10ShaderResourceView * texture = TextureManager::Instance()->LoadTexture("Media\\debug\\circle.png");

		effect->SetTexture(texture);

		effect->SetAlpha(1.0);

		// get the graphics device
		ID3D10Device * device = Graphics::GetInstance()->Device();

		// set the input layout on the device
		device->IASetInputLayout(effect->InputLayout);

		// set the vertices
		D3DXVECTOR2 tex1 = D3DXVECTOR2(0,0);
		D3DXVECTOR2 tex2 = D3DXVECTOR2(1,0);
		D3DXVECTOR2 tex3 = D3DXVECTOR2(1,1);
		D3DXVECTOR2 tex4 = D3DXVECTOR2(0,1);

		Vector3 dimensions = Vector3(radius,radius,5);

		VertexPositionTextureNormal vertices[] =
		{
			{ D3DXVECTOR3(centrePoint.X -dimensions.X/2, centrePoint.Y -dimensions.Y/2, dimensions.Z ), tex1, D3DXVECTOR3(0,1,0)}, // 0
			{ D3DXVECTOR3(centrePoint.X + dimensions.X/2,centrePoint.Y  -dimensions.Y/2, dimensions.Z ), tex2, D3DXVECTOR3(0,1,0)}, // 1
			{ D3DXVECTOR3(centrePoint.X + dimensions.X/2,centrePoint.Y + dimensions.Y/2, dimensions.Z), tex3, D3DXVECTOR3(0,1,0)},// 2
			{ D3DXVECTOR3(centrePoint.X -dimensions.X/2,centrePoint.Y + dimensions.Y/2, dimensions.Z ), tex4, D3DXVECTOR3(0,1,0)},// 3
		};

		DWORD indices[] =
		{
			2,1,3,0
		};

		ID3D10Buffer* vertex_buffer;
		ID3D10Buffer* index_buffer;

		// set vertex buffer

		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(vertices[0]) * 4;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = vertices;
		
		device->CreateBuffer( &bd, &InitData, &vertex_buffer );
		
		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		device->IASetVertexBuffers(0,1, &vertex_buffer, &stride, &offset);

		// set index buffers
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(indices[0]) * 4;       
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = indices;

		device->CreateBuffer( &bd, &InitData, &index_buffer );
		device->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
		
		// Set primitive topology
		device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		D3D10_TECHNIQUE_DESC techDesc;
		effect->CurrentTechnique->GetDesc(&techDesc);
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			effect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
			device->DrawIndexed(4, 0, 0);
		}

		effect->SetTexture(NULL);
		effect->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);
		device->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		UINT null = 0;
		ID3D10Buffer* nullB = 0;
		device->IASetVertexBuffers (0, 1, &nullB, &null, &null);

		// release buffers
		index_buffer->Release();
		vertex_buffer->Release();
	}
};

#endif
