#ifndef VERTEXTYPES_H
#define VERTEXTYPES_H

struct VertexPositionColor
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR4 Color;
};

struct VertexPositionTextureNormal
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 TexCoord;
	D3DXVECTOR3 Normal;
};

struct VertexPositionTextureNormalTanBiNorm
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 TexCoord;
	D3DXVECTOR3 Normal;
	D3DXVECTOR3 Tangent;
	D3DXVECTOR3 Binormal;
};

#endif