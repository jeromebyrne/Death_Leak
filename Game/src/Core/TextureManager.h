#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

class TextureManager
{
private:
	static TextureManager* m_instance;

	map<string, ID3D10ShaderResourceView*> m_textureIDmap;
	map<string, ID3D10ShaderResourceView*> m_textureIDmap_ui; // separate cache for UI

	ID3D10Device * m_graphicsDevice;

	TextureManager(void);
	~TextureManager(void);

public:
	
	static TextureManager* Instance();
	void Release();
	ID3D10ShaderResourceView* LoadTexture(const char * fileName);
	ID3D10ShaderResourceView* LoadTexture_ui(const char * filename); // separate function for UI texture loading
	void Initialise(ID3D10Device * graphicsDevice);
};
#endif