#ifndef SPRITE_H
#define SPRITE_H

#include "drawableobject.h"
#include "Animation.h"

class Sprite : public DrawableObject
{
public:

	Sprite(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 1.0f, float height = 1.0f);
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

	virtual void AttachTo(std::shared_ptr<GameObject> & parent, Vector3 offset, bool trackParentsOrientation = true) override;

	void SetTextureFilename(const char * fileName) { m_textureFilename = fileName; }

	virtual void DebugDraw(ID3D10Device * graphicsdevice) override;

	void SetShowBurstTint(bool value) { mShowingBurstTint = value; }

	void SetburstTintStartTime(float value) { mBurstTintStartTime = value; }

	// This is used for the animation viewer
	virtual void Sprite::UpdateAnimTexture(const string & bodyPart);

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
	void Draw_effectFoliageSway(ID3D10Device * graphicsdevice);

	void SetDimensionsAsNative();

	Vector2 GetTextureDimensions();

	// dictates texture coordinates
	bool m_horizontalFlip;
	bool m_verticalFlip;

	//animation stuff
	bool m_isAnimated;
	Animation * m_animation;
	std::string m_animationFile;
	virtual void UpdateAnimations();

	// buffers
	ID3D10Buffer* mVertexBuffer;
	ID3D10Buffer* mVertexBufferBump;
	ID3D10Buffer* mIndexBuffer;
	
	bool m_drawAtNativeDimensions;

	bool mRepeatTextureX;
	bool mRepeatTextureY;

	Vector2 mTextureDimensions;

	float mNoiseShaderIntensity;
	float mWobbleShaderIntensity;

	float mPixelWobbleIntensity;
	float mPixelWobbleSpeedMod;

	bool mParentHFlipInitial;
	bool mHflippedOnAttach;

	bool mShowingBurstTint;
	float mBurstTintStartTime;

private:

	friend class NinjaSpawner;

	virtual void UpdateToParent() override;

	struct FoliageSwayProperties
	{
		Vector2 TopLeftSwayIntensity;
		Vector2 TopRightSwayIntensity;
		Vector2 BottomLeftSwayIntensity;
		Vector2 BottomRightSwayIntensity;
		float TimeMultiplier;
		float GlobalTimeMultiplier;
	
		FoliageSwayProperties()
		{
			TimeMultiplier = 1.0f;
		}
	};

	FoliageSwayProperties mFoliageSwayProperties;
};

#endif
