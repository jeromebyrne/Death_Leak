#ifndef SCREENALIGNEDTEXTURE_H
#define SCREENALIGNEDTEXTURE_H

// class which draws a texture in the middle of the screen (used for post processing)
class ScreenAlignedTexture
{
public:
	ScreenAlignedTexture(void);

	~ScreenAlignedTexture(void);

	void Initialise();

	void Draw();

	void SetEffect(EffectAbstract * effect)
	{
		m_effect = effect;
	}

	void SetDimensions(int width, int height)
	{
		m_dimensions.X = width;
		m_dimensions.Y = height;

		if (VertexBuffer) // only do this if the vertex buffer has already been initialised
		{
			SetVertexBuffer(Graphics::GetInstance()->Device(), sizeof(m_vertices[0]) * 4, m_vertices);
		}
	}

	void Release();

private:

	D3DXMATRIX m_world, m_view, m_projection;

	EffectAbstract * m_effect;

	VertexPositionTextureNormal m_vertices[4];
	DWORD  m_indices[4];

	Vector2 m_dimensions;
	
	void SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[]);
	void SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[]);

	// buffers
	ID3D10Buffer* VertexBuffer;
	ID3D10Buffer* IndexBuffer;
};

#endif
