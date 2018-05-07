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
class EffectFoliageSway;

class DrawableObject : public GameObject
{
public:
	DrawableObject(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1, const char* effectName = "effectlighttexture");
	virtual ~DrawableObject(void);

	std::string EffectName;
	virtual void Update(float delta) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void Initialise() override;
	virtual void ApplyChange(ID3D10Device * device) {}
	bool IsChangeRequired() { return m_applyChange; }
	void SetChangeRequired() { m_applyChange = true; }
	D3DXMATRIX World() { return m_world; }

	void SetFadeAlphaWhenPlayerOccluded(bool value, float alphaWhenOccluding)
	{
		m_fadeAlphaWhenPlayerOccluded = value;
		m_alphaWhenOccluding = alphaWhenOccluding;
	}

	virtual void SetAlpha(float value)
	{
		if (value > 1.0f)
		{
			value = 1.0f;
		}
		else if (value < 0.0f)
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

protected:

	// NOTE: all drawable objects have access to all shaders, they may choose to use more than one
	EffectBasic * m_effectBasic;
	EffectLightTexture * m_effectLightTexture;
	EffectLightTextureVertexWobble * m_effectLightTextureWobble;
	EffectReflection * m_effectReflection;
	EffectLightTextureBump * m_effectLightTextureBump;
	EffectNoise * m_effectNoise;
	EffectLightTexturePixelWobble * m_effectPixelWobble;
	EffectFoliageSway * m_effectFoliageSway;

	enum EffectTypesEnum m_currentEffectType;
	float m_alpha;
	float mOriginalAlpha;
	bool m_applyChange;

	bool m_fadeAlphaWhenPlayerOccluded;
	float m_alphaWhenOccluding;
	float m_alphaWhenNotOccluding;
};

#endif
