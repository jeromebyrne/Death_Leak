#include "precompiled.h"
#include "DrawableObject.h"
#include "xmlutilities.h"
#include "EffectBasic.h"
#include "EffectLightTexture.h"
#include "EffectLightTextureVertexWobble.h"
#include "EffectReflection.h"
#include "EffectParticleSpray.h"
#include "EffectLightTextureBump.h"
#include "EffectNoise.h"
#include "EffectLightTexturePixelWobble.h"

DrawableObject::DrawableObject(float x, float y, float z, float width, float height, float breadth, const char * effectName): GameObject(x,y,z,width,height,breadth),
EffectName(effectName), 
m_applyChange(false), 
m_alpha(1),
m_effectBasic(nullptr),
m_effectLightTexture(nullptr),
m_effectLightTextureWobble(nullptr),
m_effectLightTextureBump(nullptr),
m_effectNoise(nullptr)
{
	LOG_INFO("Refactor DrawableObject constructor");
	// GameObjectManager::Instance()->AddDrawableObject(this);
	m_currentEffectType = EFFECT_LIGHT_TEXTURE; // set default effect
	mDrawable = true;
}

DrawableObject::~DrawableObject(void)
{
}

void DrawableObject::Update(float delta)
{
	GameObject::Update(delta);
}

void DrawableObject::LoadContent(ID3D10Device * graphicsdevice)
{
	// get our effects
	m_effectBasic = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));
	m_effectLightTexture = static_cast<EffectLightTexture*>(EffectManager::Instance()->GetEffect("effectlighttexture"));
	m_effectLightTextureWobble = static_cast<EffectLightTextureVertexWobble*>(EffectManager::Instance()->GetEffect("effectlighttexturevertexwobble"));
	m_effectReflection = static_cast<EffectReflection*>(EffectManager::Instance()->GetEffect("effectreflection"));
	m_effectLightTextureBump = static_cast<EffectLightTextureBump*>(EffectManager::Instance()->GetEffect("effectlighttexturebump"));
	m_effectNoise = static_cast<EffectNoise*>(EffectManager::Instance()->GetEffect("effectnoise"));
	m_effectPixelWobble = static_cast<EffectLightTexturePixelWobble*>(EffectManager::Instance()->GetEffect("effectpixelwobble"));

	//set our effect enum
	if(EffectName == "effectbasic")
	{
		m_currentEffectType = EFFECT_BASIC;
	}
	else if(EffectName == "effectlighttexture")
	{
		m_currentEffectType = EFFECT_LIGHT_TEXTURE;
	}
	else if (EffectName == "effectlighttexturebump")
	{
		m_currentEffectType = EFFECT_BUMP;
	}
	else if(EffectName == "effectlighttexturevertexwobble")
	{
		m_currentEffectType = EFFECT_VERTEX_WOBBLE;
	}
	else if(EffectName == "effectreflection")
	{
		m_currentEffectType = EFFECT_REFLECT;
	}
	else if (EffectName == "effectnoise")
	{
		m_currentEffectType = EFFECT_NOISE;
	}
	else if (EffectName == "effectpixelwobble")
	{
		m_currentEffectType = EFFECT_PIXEL_WOBBLE;
	}
}

void DrawableObject::XmlRead(TiXmlElement * element)
{
	// read the base object properties first
	GameObject::XmlRead(element);
	
	// read the alpha value
	m_alpha = XmlUtilities::ReadAttributeAsFloat(element, "alpha", "value");

	// go to the effect node
	// this is used to determine what effect we will use to draw our object
	EffectName = XmlUtilities::ReadAttributeAsString(element, "effect", "name");
}

void DrawableObject::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * alphaElem = new TiXmlElement("alpha");
	alphaElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_alpha).c_str());
	element->LinkEndChild(alphaElem);

	TiXmlElement * effectElem = new TiXmlElement("effect");
	effectElem->SetAttribute("name", EffectName.c_str());
	element->LinkEndChild(effectElem);
}

void DrawableObject::Scale(float xScale, float yScale, bool scalePosition)
{
	GameObject::Scale(xScale, yScale, scalePosition);
	// we're scaling swo must need to change some stuff
	m_applyChange = true;
}
