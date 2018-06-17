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
#include "EffectFoliageSway.h"
#include "Game.h"

static const float kOcclusionFadeOutMultiplier = 1.5f;

DrawableObject::DrawableObject(float x, float y, DepthLayer depthLayer, float width, float height, const char * effectName): 
		GameObject(x,y, depthLayer, width,height),
	EffectName(effectName), 
	m_applyChange(false), 
	m_alpha(1.0f),
	m_effectBasic(nullptr),
	m_effectLightTexture(nullptr),
	m_effectLightTextureWobble(nullptr),
	m_effectLightTextureBump(nullptr),
	m_effectNoise(nullptr),
	m_effectFoliageSway(nullptr),
	mOriginalAlpha(1.0f),
	m_fadeAlphaWhenPlayerOccluded(false),
	m_alphaWhenOccluding(0.5f),
	m_alphaWhenNotOccluding(1.0f)
{
	m_currentEffectType = EFFECT_LIGHT_TEXTURE; // set default effect
	mDrawable = true;
}

DrawableObject::~DrawableObject(void)
{
}

void DrawableObject::Update(float delta)
{
	GameObject::Update(delta);

#ifdef DEBUG

	if (Game::GetIsLevelEditMode())
	{
		// always show text if in level editor mode, otherwise it's... annoying
		return;
	}
#endif

	if (m_fadeAlphaWhenPlayerOccluded)
	{
		// get the distance to the player
		const Player * player = GameObjectManager::Instance()->GetPlayer();

		if (player && player != this && GetDepthLayer() < player->GetDepthLayer())
		{
			// check is the player inside the bounds of this sprite
			// if so then fade alpha
			if (player->X() > Left() &&
				player->X() < Right() &&
				player->Y() > Bottom() &&
				player->Y() < Top())
			{
				if (m_alphaWhenOccluding < m_alphaWhenNotOccluding)
				{
					if (m_alpha > m_alphaWhenOccluding)
					{
						m_alpha -= kOcclusionFadeOutMultiplier * delta;
					}
					else
					{
						m_alpha = m_alphaWhenOccluding;
					}
				}
				else
				{
					if (m_alpha < m_alphaWhenOccluding)
					{
						m_alpha += kOcclusionFadeOutMultiplier * delta;
					}
					else
					{
						m_alpha = m_alphaWhenOccluding;
					}
				}
				
			}
			else
			{
				if (m_alphaWhenOccluding < m_alphaWhenNotOccluding)
				{
					if (m_alpha < m_alphaWhenNotOccluding)
					{
						m_alpha += kOcclusionFadeOutMultiplier * delta;
					}
					else
					{
						m_alpha = m_alphaWhenNotOccluding;
					}
				}
				else
				{
					if (m_alpha > m_alphaWhenNotOccluding)
					{
						m_alpha -= kOcclusionFadeOutMultiplier * delta;
					}
					else
					{
						m_alpha = m_alphaWhenNotOccluding;
					}
				}
			}
		}
	}
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
	m_effectFoliageSway = static_cast<EffectFoliageSway*>(EffectManager::Instance()->GetEffect("effectfoliagesway"));

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
	else if (EffectName == "effectfoliagesway")
	{
		m_currentEffectType = EFFECT_FOLIAGE_SWAY;
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

	m_fadeAlphaWhenPlayerOccluded = XmlUtilities::ReadAttributeAsBool(element, "fade_when_player_occluded", "value");
	m_alphaWhenOccluding = XmlUtilities::ReadAttributeAsFloat(element, "fade_when_player_occluded", "alphawhenoccluding");
	m_alphaWhenNotOccluding = XmlUtilities::ReadAttributeAsFloat(element, "fade_when_player_occluded", "alphawhennotoccluding");
}

void DrawableObject::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * alphaElem = new TiXmlElement("alpha");
	alphaElem->SetDoubleAttribute("value", mOriginalAlpha);
	element->LinkEndChild(alphaElem);

	TiXmlElement * effectElem = new TiXmlElement("effect");
	effectElem->SetAttribute("name", EffectName.c_str());
	element->LinkEndChild(effectElem);

	const char * ifade_when_player_occludedAsStr = m_fadeAlphaWhenPlayerOccluded ? "true" : "false";
	TiXmlElement * fade_when_player_occluded = new TiXmlElement("fade_when_player_occluded");
	fade_when_player_occluded->SetAttribute("value", ifade_when_player_occludedAsStr);

	fade_when_player_occluded->SetDoubleAttribute("alphawhenoccluding", m_alphaWhenOccluding);
	fade_when_player_occluded->SetDoubleAttribute("alphawhennotoccluding", m_alphaWhenNotOccluding);

	element->LinkEndChild(fade_when_player_occluded);
}

void DrawableObject::Scale(float xScale, float yScale, bool scalePosition)
{
	GameObject::Scale(xScale, yScale, scalePosition);
	// we're scaling swo must need to change some stuff
	m_applyChange = true;
}

void DrawableObject::Initialise()
{
	GameObject::Initialise();

	mOriginalAlpha = m_alpha;
}
