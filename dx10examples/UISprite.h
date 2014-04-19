#ifndef UISPRITE_H
#define UISPRITE_H

#include "uiwidget.h"

class UISprite : public UIWidget
{
protected:

	ID3D10ShaderResourceView* m_texture;
	string m_textureFilename;
	
	// four points on a sprite (1 quad)
	VertexPositionTextureNormal m_vertices[4];
	DWORD  m_indices[4];

	void SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[]);
	void SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[]);

	// buffers
	ID3D10Buffer* VertexBuffer;
	ID3D10Buffer* IndexBuffer;

	bool m_horizontalFlip;
	bool m_verticalFlip;

	// does a change need to be made?
	bool m_applyChange;

	D3DXMATRIX m_world; // world matrix
public:
	UISprite(void);
	virtual ~UISprite(void);
	virtual void Release();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
	virtual void Draw(ID3D10Device * graphicsdevice);
	virtual void Initialise();

	virtual void Reset();
};

#endif
