#ifndef CUBE_H
#define CUBE_H
#define CUBE_NUMVERTICES 24
#define CUBE_NUMINDICES 36

class Cube
{
public:

private:
	
	D3DXMATRIX m_world; // world matrix

	D3DXVECTOR3 m_dimensions;
	VertexPositionTextureNormal m_vertices[CUBE_NUMVERTICES];
	DWORD  m_indices[CUBE_NUMINDICES];

	
	
public:

	// buffers
	ID3D10Buffer* VertexBuffer;
	ID3D10Buffer* IndexBuffer;

	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_orientation;
	D3DXMATRIX rotation;

	Cube(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1);
	~Cube(void);
	
	const int NUM_VERTEX;
	const int NUM_INDEX;

	void Update(D3DXMATRIX globalWorld);

	void Initialise(ID3D10Device* device);

	inline VertexPositionTextureNormal* Vertices()
	{
		return m_vertices;
	}
	
	inline DWORD* Indices()
	{
		return m_indices;
	}

	inline D3DXMATRIX & World()
	{
		return m_world;
	}

	void SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[]);
	void SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[]);

	

};

#endif