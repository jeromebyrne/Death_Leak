#ifndef SPRITE_H
#define SPRITE_H

#include "drawableobject.h"
#include "Animation.h"

class ParticleSpray;

class Sprite : public DrawableObject
{
private:

	// if we are attched to another sprite then update the logic for atatching
	virtual void UpdateToParent();

	friend class NinjaSpawner;

protected:
	ID3D10ShaderResourceView* m_texture;
	std::string m_textureFilename;

	ID3D10ShaderResourceView* m_textureBump;
	std::string m_textureBumpFilename;
	
	// for points on a sprite (1 quad)
	VertexPositionTextureNormal m_vertices[4];
	VertexPositionTextureNormalTanBiNorm m_verticesBump[4];
	DWORD  m_indices[4];

	void SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[]);
	void SetVertexBufferBump(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormalTanBiNorm vertices[]);
	void SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[]);
	void CalculateTangentBinormal(VertexPositionTextureNormalTanBiNorm vertices[], Vector3 &tangent, Vector3 &binormal);
	void CalculateNormal(Vector3 tangent, Vector3 binormal, Vector3 &normal);

	// our draw functions
	void Draw_effectBasic(ID3D10Device * graphicsdevice);
	void Draw_effectLightTexture(ID3D10Device * graphicsdevice);
	void Draw_effectLightTexWobble(ID3D10Device * graphicsdevice);
	void Draw_effectReflection(ID3D10Device * graphicsdevice);
	void Draw_effectBump(ID3D10Device * graphicsdevice);
	void Draw_effectNoise(ID3D10Device * graphicsdevice);
	void Draw_effectPixelWobble(ID3D10Device * graphicsdevice);

	void SetDimensionsAsNative();
	void NotifyParticlesDetach();

	Vector2 GetTextureDimensions();

	// dictates texture coordinates
	bool m_horizontalFlip;
	bool m_verticalFlip;

	//animation stuff
	bool m_isAnimated;
	Animation * m_animation;
	std::string m_animationFile;
	virtual void UpdateAnimations();

	Sprite * m_attachedTo; // attached to this object
	Vector3 m_attachedToOffset; 
	bool m_parentHFlipInitial; // was the parent flipped on the initial attach
	bool m_hflippedOnAttach; // were we h-flipped when we first attached

	list<ParticleSpray *> mParticlesAttachedToUs;

	// buffers
	ID3D10Buffer* VertexBuffer;
	ID3D10Buffer* VertexBufferBump;
	ID3D10Buffer* IndexBuffer;
	
	bool m_drawAtNativeDimensions;

	bool m_fadeAlphaWhenPlayerOccluded;
	float m_alphaWhenOccluding;
	float m_alphaWhenNotOccluding;

	bool mRepeatTextureX;
	bool mRepeatTextureY;

	Vector2 mTextureDimensions;

	float mNoiseShaderIntensity;
	float mWobbleShaderIntensity;

	float mPixelWobbleIntensity;
	float mPixelWobbleSpeedMod;

public:
	Sprite(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1);
	virtual ~Sprite(void);

	virtual void Initialise() override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	virtual void Update(float delta) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void ApplyChange(ID3D10Device * device) override;

	bool GetDoesRepeatX() { return mRepeatTextureX; } 
	bool GetDoesRepeatY() { return mRepeatTextureY; } 

	// updates another objects position 
	void AttachToSprite(Sprite * parent, Vector3 offset);
	void DetachFromSprite();

	void SetNoiseSHaderIntensity(float value) { mNoiseShaderIntensity = value; }

	Animation * GetAnimation();
	bool GetIsAnimated()
	{
		return m_isAnimated;
	}
	
	// these functions determine texture coords
	void FlipHorizontal();
	void UnFlipHorizontal();
	void FlipVertical();
	void UnFlipVertical();

	bool IsHFlipped() const { return m_horizontalFlip; }
	bool IsVFlipped() const { return m_verticalFlip; }

	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;

	// should only be calling this in level editor code, not in-game
	void ScaleSpriteOnly(float xScale, float yScale);

	void SetIsNativeDimensions(bool value);

	void RecalculateVertices();

	void OnParticleAttached(ParticleSpray * spray);
	void OnParticleSprayDead(ParticleSpray * spray);
};

#endif