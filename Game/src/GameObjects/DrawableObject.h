#ifndef DRAWABLEOBJECT_H
#define DRAWABLEOBJECT_H

#include "gameobject.h"

class Camera2D;
class EffectBasic;
class EffectLightTexture;
class EffectLightTextureVertexWobble;
class EffectReflection;
class EffectLightTextureBump;
class EffectNoise;
class EffectLightTexturePixelWobble;

class DrawableObject : public GameObject
{
protected:
	// NOTE: all drawable objects have access to all shaders, they may choose to use more than one
	EffectBasic * m_effectBasic;
	EffectLightTexture * m_effectLightTexture;
	EffectLightTextureVertexWobble * m_effectLightTextureWobble;
	EffectReflection * m_effectReflection;
	EffectLightTextureBump * m_effectLightTextureBump;
	EffectNoise * m_effectNoise;
	EffectLightTexturePixelWobble * m_effectPixelWobble;

	// determines our effect type
	enum EffectTypesEnum m_currentEffectType;

	// the overall alpha value of the object
	float m_alpha;

	// does a change need to be made?
	bool m_applyChange;

public:
	DrawableObject(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1, char* effectName = "effectlighttexture");
	virtual ~DrawableObject(void);
	
	std::string EffectName;
	virtual void Initialise();
	virtual void Update(float delta) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera);
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
	virtual void XmlWrite(TiXmlElement * element);
	virtual void ApplyChange(ID3D10Device * device) {} // apply any render changes
	bool IsChangeRequired() // do we need to make a change
	{
		return m_applyChange;
	}
	D3DXMATRIX World() // the graphical views world matrix
	{
		return m_world;
	}
	void SetAlpha(float value)
	{
		if(value > 1.0f)
		{
			value = 1.0f;
		}
		else if(value < 0.0f)
		{
			value = 0.0f;
		}
	
		m_alpha = value;
	}
	inline float Alpha()
	{
		return m_alpha;
	}

	virtual void Scale(float xScale, float yScale, bool scalePosition = true);
};

#endif
