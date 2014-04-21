#ifndef MESH_H
#define MESH_H

class Mesh
{
private:
	CDXUTSDKMesh m_mesh;

	D3DXMATRIX m_world; // world matrix
public:
	Mesh(void);
	~Mesh(void);
	inline D3DXMATRIX World()
	{
		return m_world;
	}
	void Load(wchar_t* fileName, ID3D10Device * graphicsDevice);
	void Update(D3DXMATRIX globalWorld);
	void Draw(ID3D10Device * graphicsDevice, ID3D10EffectTechnique* technique, ID3D10EffectShaderResourceVariable* shaderResourceVar);
};

#endif
