#include "precompiled.h"
#include "Sprite.h"
#include "xmlutilities.h"
#include <algorithm>
#include "ParticleSpray.h"
#include "EffectBasic.h"
#include "EffectLightTexture.h"
#include "EffectLightTextureVertexWobble.h"
#include "EffectReflection.h"
#include "EffectParticleSpray.h"
#include "EffectLightTextureBump.h"
#include "effectNoise.h"
#include "EffectLightTexturePixelWobble.h"
#include "EffectFoliageSway.h"
#include "DrawUtilities.h"

static const float kBurstTintMaxTime = 0.8f;

static const D3DXVECTOR2 kDefaultTex1 = D3DXVECTOR2(0,0);
static const D3DXVECTOR2 kDefaultTex2 = D3DXVECTOR2(1,0);
static const D3DXVECTOR2 kDefaultTex3 = D3DXVECTOR2(1,1);
static const D3DXVECTOR2 kDefaultTex4 = D3DXVECTOR2(0,1);

static const D3DXVECTOR3 kDefaultNormal = D3DXVECTOR3(0,0,1);
static const D3DXVECTOR3 kDefaultBumpNormal = D3DXVECTOR3(0,0,-1);
static const D3DXVECTOR3 kDefaultTangent = D3DXVECTOR3(0,1,0);
static const D3DXVECTOR3 kDefaultBiNormal = D3DXVECTOR3(1,1,1);

Sprite::Sprite(float x, float y, float z, float width, float height, float breadth)
	:DrawableObject(x,y,z,width,height,breadth),
	m_horizontalFlip(false),
	m_verticalFlip(false), 
	mVertexBuffer(nullptr), 
	mVertexBufferBump(nullptr),
	mIndexBuffer(nullptr),
	m_isAnimated(false),
	mParentHFlipInitial(false),
	mHflippedOnAttach(true),
	m_drawAtNativeDimensions(true),
	m_texture(nullptr),
	m_textureBump(nullptr),
	mRepeatTextureX(false),
	mRepeatTextureY(false),
	mTextureDimensions(0,0),
	mNoiseShaderIntensity(0.0f),
	mWobbleShaderIntensity(0.0f),
	mPixelWobbleIntensity(0.0f),
	mPixelWobbleSpeedMod(0.0f),
	mShowingBurstTint(false),
	mBurstTintStartTime(0.0f)
{
}

Sprite::~Sprite(void)
{
	if(mVertexBuffer) 
	{
		mVertexBuffer->Release();
		mVertexBuffer = nullptr;
	}
	if(mVertexBufferBump) 
	{
		mVertexBufferBump->Release();
		mVertexBufferBump = nullptr;
	}
	if(mIndexBuffer)
	{
		mIndexBuffer->Release();
		mIndexBuffer = nullptr;
	}
}

void Sprite::Initialise()
{
	DrawableObject::Initialise();

	if(m_isAnimated) // if we are using animations
	{
		// TODO: don't read this from xml all of the time,
		// Just read once and load from internal structure
		m_animation = new Animation(m_animationFile.c_str());
	}

	if (m_drawAtNativeDimensions)
	{
		SetDimensionsAsNative();
	}

	if (m_horizontalFlip)
	{
		m_matScaleX *= -1;
	}
	if (m_verticalFlip)
	{
		m_matScaleY *= -1;
	}

	mTextureDimensions = GetTextureDimensions();
	if (m_isAnimated && m_animation)
	{
		float scaleX = m_dimensions.X / mTextureDimensions.X;

		m_animation->ScaleSkeleton(scaleX);
	}
}

Vector2 Sprite::GetTextureDimensions()
{
	if (!IsDrawable() && m_textureFilename.empty())
	{
		// it's okay if a non drawable object doesn't have a texture (see SolidLineStrip)
		return Vector2(0, 0);
	}

	ID3D10ShaderResourceView * srv; 
	if (m_isAnimated)
	{
		m_animation->SetPartSequence("body", "Still");
		AnimationPart * body_anim = m_animation->GetPart("body");

		if (body_anim)
		{
			srv = body_anim->CurrentFrame();
		}
		else
		{
			srv = TextureManager::Instance()->LoadTexture(m_textureFilename.c_str());
		}
	}
	else
	{
		srv = TextureManager::Instance()->LoadTexture(m_textureFilename.c_str());
	}
	ID3D10Texture2D * tex2d;
	srv->GetResource( reinterpret_cast<ID3D10Resource**>(&tex2d));

	D3D10_TEXTURE2D_DESC desc;
	tex2d->GetDesc(&desc);

	return Vector2(desc.Width, desc.Height);
}

void Sprite::SetDimensionsAsNative()
{
	Vector2 texDimensions = GetTextureDimensions();
	m_dimensions.X = texDimensions.X;
	m_dimensions.Y = texDimensions.Y;

	m_drawAtNativeDimensions = true;
}

void Sprite::SetIsNativeDimensions(bool value)
{
	if (value == true && m_drawAtNativeDimensions == false)
	{
		SetDimensionsAsNative();
	}

	m_drawAtNativeDimensions = value;
}

void Sprite::LoadContent(ID3D10Device * graphicsdevice)
{
	DrawableObject::LoadContent(graphicsdevice);

	// load the sprite texture
	if (!m_isAnimated)
	{
		m_texture = TextureManager::Instance()->LoadTexture(m_textureFilename.c_str());
	}
	
	if (m_currentEffectType == EFFECT_BUMP)
	{
		m_textureBump = TextureManager::Instance()->LoadTexture(m_textureBumpFilename.c_str());
	}
	
	ID3D10Device * device = Graphics::GetInstance()->Device();

	RecalculateVertices();

	DWORD indices[] =
    {
        2,1,3,0
    };

	for(int i = 0; i < 4; i++)
	{
		m_indices[i] = indices[i];
	}

	SetIndexBuffer(device, sizeof(m_indices[0]) * 4, m_indices);
}

void Sprite::RecalculateVertices()
{
	ID3D10Device * device = Graphics::GetInstance()->Device();

	if (m_currentEffectType != EFFECT_BUMP)
	{
		D3DXVECTOR3 normal1 = kDefaultNormal;
		D3DXVECTOR3 normal2 = kDefaultNormal;
		D3DXVECTOR3 normal3 = kDefaultNormal;
		D3DXVECTOR3 normal4 = kDefaultNormal;

		if (m_currentEffectType == EFFECT_FOLIAGE_SWAY)
		{
			normal1 = D3DXVECTOR3(mFoliageSwayProperties.BottomLeftSwayIntensity.X, mFoliageSwayProperties.BottomLeftSwayIntensity.Y, 1);
			normal2 = D3DXVECTOR3(mFoliageSwayProperties.BottomRightSwayIntensity.X, mFoliageSwayProperties.BottomRightSwayIntensity.Y, 1);
			normal3 = D3DXVECTOR3(mFoliageSwayProperties.TopRightSwayIntensity.X, mFoliageSwayProperties.TopRightSwayIntensity.Y, 1);
			normal4 = D3DXVECTOR3(mFoliageSwayProperties.TopLeftSwayIntensity.X, mFoliageSwayProperties.TopLeftSwayIntensity.Y, 1);
		}

		VertexPositionTextureNormal vertices[] =
		{
			{ D3DXVECTOR3( -m_dimensions.X/2, -m_dimensions.Y/2, m_dimensions.Z ), kDefaultTex1, normal1 }, // 0
			{ D3DXVECTOR3( m_dimensions.X/2, -m_dimensions.Y/2, m_dimensions.Z ), kDefaultTex2, normal2 }, // 1
			{ D3DXVECTOR3( m_dimensions.X/2, m_dimensions.Y/2, m_dimensions.Z), kDefaultTex3, normal3 },// 2
			{ D3DXVECTOR3( -m_dimensions.X/2, m_dimensions.Y/2, m_dimensions.Z ), kDefaultTex4, normal4 },// 3
		};

		for(int i = 0; i < 4; i++)
		{
			m_vertices[i] = vertices[i];
		}

		if (mRepeatTextureX && !m_drawAtNativeDimensions)
		{
			m_vertices[1].TexCoord.x = m_dimensions.X / mTextureDimensions.X;
			m_vertices[2].TexCoord.x = m_dimensions.X / mTextureDimensions.X;
		}
	}
	else
	{
		VertexPositionTextureNormalTanBiNorm vertices[]=
		{
			{ D3DXVECTOR3( -m_dimensions.X/2, -m_dimensions.Y/2, m_dimensions.Z ), kDefaultTex1, kDefaultBumpNormal, kDefaultTangent, kDefaultBiNormal}, // 0
			{ D3DXVECTOR3( m_dimensions.X/2, -m_dimensions.Y/2, m_dimensions.Z ), kDefaultTex2, kDefaultBumpNormal, kDefaultTangent, kDefaultBiNormal}, // 1
			{ D3DXVECTOR3( m_dimensions.X/2, m_dimensions.Y/2, m_dimensions.Z), kDefaultTex3, kDefaultBumpNormal, kDefaultTangent, kDefaultBiNormal},// 2
			{ D3DXVECTOR3( -m_dimensions.X/2, m_dimensions.Y/2, m_dimensions.Z ), kDefaultTex4, kDefaultBumpNormal, kDefaultTangent, kDefaultBiNormal},// 3
		};

		Vector3 tangent;
		Vector3 binormal;
		Vector3 normal;

		CalculateTangentBinormal(vertices, tangent, binormal);
		CalculateNormal(tangent, binormal, normal);

		for(int i = 0; i < 4; i++)
		{
			m_verticesBump[i] = vertices[i];

			m_verticesBump[i].Normal.x = normal.X;
			m_verticesBump[i].Normal.y = normal.Y;
			m_verticesBump[i].Normal.z = normal.Z;
			m_verticesBump[i].Tangent.x = tangent.X;
			m_verticesBump[i].Tangent.y = tangent.Y;
			m_verticesBump[i].Tangent.z = tangent.Z;
			m_verticesBump[i].Binormal.x = binormal.X;
			m_verticesBump[i].Binormal.y = binormal.Y;
			m_verticesBump[i].Binormal.z = binormal.Z;
		}

		if (mRepeatTextureX && !m_drawAtNativeDimensions)
		{
			m_verticesBump[1].TexCoord.x = m_dimensions.X / mTextureDimensions.X;
			m_verticesBump[2].TexCoord.x = m_dimensions.X / mTextureDimensions.X;
		}
	}

#ifdef _DEBUG
	SetupDebugDraw();
#endif
}

void Sprite::DebugDraw(ID3D10Device * graphicsdevice)
{
	DrawableObject::DebugDraw(graphicsdevice);

	if (!IsDrawable())
	{
		DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), Vector2(GetLevelEditSelectionDimensions().X, GetLevelEditSelectionDimensions().Y), "Media\\editor\\not_drawable.png");
	}
}

void Sprite::Scale(float xScale, float yScale, bool scalePosition)
{
	mTextureDimensions.X *= xScale;
	mTextureDimensions.Y *= yScale;

	DrawableObject::Scale(xScale, yScale, scalePosition);
	
	RecalculateVertices();
}

void Sprite::ScaleSpriteOnly(float xScale, float yScale)
{	
	Sprite::Scale(xScale, yScale, false);
}

void Sprite::FlipHorizontal()
{
	if(!m_horizontalFlip)
	{
		m_matScaleX *= -1;
		m_horizontalFlip = true;
	}
}

void Sprite::UnFlipHorizontal()
{
	if(m_horizontalFlip)
	{
		m_matScaleX *= -1;
		m_horizontalFlip = false;
	}
}

void Sprite::FlipVertical()
{
	if(!m_verticalFlip)
	{
		m_matScaleY *= -1;
		m_verticalFlip = true;
	}
}

void Sprite::UnFlipVertical()
{
	if(m_verticalFlip)
	{
		m_matScaleY *= -1;
		m_verticalFlip = false;
	}
}

void Sprite::ApplyChange(ID3D10Device * device)
{
	if (m_currentEffectType != EFFECT_BUMP)
	{
		SetVertexBuffer(device, sizeof(m_vertices[0]) * 4, m_vertices);
	}
	else
	{
		SetVertexBufferBump(device, sizeof(m_verticesBump[0]) * 4, m_verticesBump);
	}

	m_applyChange = false;
}

void Sprite::Update(float delta)
{
	DrawableObject::Update(delta);

	// update our animations
	if (m_isAnimated)
	{
		UpdateAnimations();
	}
}

void Sprite::Draw(ID3D10Device * device, Camera2D * camera)
{
	// draw base first
	DrawableObject::Draw(device, camera);

	switch(m_currentEffectType)
	{
		case EFFECT_LIGHT_TEXTURE:
			{
				Draw_effectLightTexture(device);
				break;
			}
		case EFFECT_BUMP:
			{
				Draw_effectBump(device);
				break;
			}
		case EFFECT_PIXEL_WOBBLE:
			{
				Draw_effectPixelWobble(device);
				break;
			}
		case EFFECT_VERTEX_WOBBLE:
			{
				Draw_effectLightTexWobble(device);
				break;
			}
		case EFFECT_FOLIAGE_SWAY:
			{
				Draw_effectFoliageSway(device);
				break;
			}
		case EFFECT_NOISE:
			{
				Draw_effectNoise(device);
				break;
			}
		case EFFECT_REFLECT:
			{
				Draw_effectReflection(device);
				break;
			}
		case EFFECT_BASIC:
			{
				Draw_effectBasic(device);
				break;
			}
		default:
			{
				break;
			}
	};
}

void Sprite::XmlRead(TiXmlElement * element)
{
	DrawableObject::XmlRead(element);
	
	m_textureFilename = XmlUtilities::ReadAttributeAsString(element, "texture", "filename");
	m_textureBumpFilename = m_textureFilename;
	std::string prefix = m_textureBumpFilename.substr(0, m_textureBumpFilename.find("."));
	m_textureBumpFilename = prefix + "_bump.png"; // bump maps are always png

	m_horizontalFlip = XmlUtilities::ReadAttributeAsBool(element, "horizontalflip", "value");
	m_verticalFlip = XmlUtilities::ReadAttributeAsBool(element, "verticalflip", "value");

	m_isAnimated = XmlUtilities::ReadAttributeAsBool(element, "isanimated", "value");

	if(m_isAnimated)
	{
		m_animationFile = XmlUtilities::ReadAttributeAsString(element, "animationfile", "value");
	}

	m_drawAtNativeDimensions = XmlUtilities::ReadAttributeAsBool(element, "dimensions", "native");

	if (XmlUtilities::AttributeExists(element, "dimensions", "repeatX"))
	{
		mRepeatTextureX = XmlUtilities::ReadAttributeAsBool(element, "dimensions", "repeatX");
	}

	if (XmlUtilities::AttributeExists(element, "dimensions", "repeatY"))
	{
		mRepeatTextureY = XmlUtilities::ReadAttributeAsBool(element, "dimensions", "repeatY");
	}

	if (EffectName == "effectnoise")
	{
		mNoiseShaderIntensity = XmlUtilities::ReadAttributeAsFloat(element, "effect", "noiseintensity");
	}
	else if (EffectName == "effectlighttexturevertexwobble")
	{
		mWobbleShaderIntensity = XmlUtilities::ReadAttributeAsFloat(element, "effect", "wobbleintensity");
	}
	else if (EffectName == "effectpixelwobble")
	{
		mPixelWobbleIntensity = XmlUtilities::ReadAttributeAsFloat(element, "effect", "pixelwobbleintensity");
		mPixelWobbleSpeedMod = XmlUtilities::ReadAttributeAsFloat(element, "effect", "pixelwobblespeedmod");
	}
	else if (EffectName == "effectfoliagesway")
	{
		mFoliageSwayProperties.BottomLeftSwayIntensity.X = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "bottom_left_intensity_x");
		mFoliageSwayProperties.BottomLeftSwayIntensity.Y = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "bottom_left_intensity_y");
		mFoliageSwayProperties.BottomRightSwayIntensity.X = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "bottom_right_intensity_x");
		mFoliageSwayProperties.BottomRightSwayIntensity.Y = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "bottom_right_intensity_y");
		mFoliageSwayProperties.TopLeftSwayIntensity.X = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "top_left_intensity_x");
		mFoliageSwayProperties.TopLeftSwayIntensity.Y = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "top_left_intensity_y");
		mFoliageSwayProperties.TopRightSwayIntensity.X = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "top_right_intensity_x");
		mFoliageSwayProperties.TopRightSwayIntensity.Y = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "top_right_intensity_y");
		mFoliageSwayProperties.TimeMultiplier = XmlUtilities::ReadAttributeAsFloat(element, "foliage_sway_props", "time_multiplier");
	}
}

void Sprite::XmlWrite(TiXmlElement * element)
{
	DrawableObject::XmlWrite(element);

	TiXmlElement * texture = new TiXmlElement("texture");
	texture->SetAttribute("filename", m_textureFilename.c_str());
	element->LinkEndChild(texture);

	const char * horizontalflipAsStr = m_horizontalFlip ? "true" : "false";
	TiXmlElement * horizontalflip = new TiXmlElement("horizontalflip");
	horizontalflip->SetAttribute("value", horizontalflipAsStr);
	element->LinkEndChild(horizontalflip);

	const char * verticalflipAsStr = m_verticalFlip ? "true" : "false";
	TiXmlElement * verticalflip = new TiXmlElement("verticalflip");
	verticalflip->SetAttribute("value", verticalflipAsStr);
	element->LinkEndChild(verticalflip);

	const char * isanimatedAsStr = m_isAnimated ? "true" : "false";
	TiXmlElement * isanimated = new TiXmlElement("isanimated");
	isanimated->SetAttribute("value", isanimatedAsStr);
	element->LinkEndChild(isanimated);

	if (m_isAnimated)
	{
		TiXmlElement * animationfile = new TiXmlElement("animationfile");
		animationfile->SetAttribute("value", m_animationFile.c_str());
		element->LinkEndChild(animationfile);
	}

	TiXmlElement * dimensionsElem = XmlUtilities::GetChildElement(element, "dimensions");
	const char * nativeAsStr = m_drawAtNativeDimensions ? "true" : "false";
	dimensionsElem->SetAttribute("native", nativeAsStr);
	const char * repeatXAsStr = mRepeatTextureX ? "true" : "false";
	dimensionsElem->SetAttribute("repeatX",  repeatXAsStr);
	const char * repeatYAsStr = mRepeatTextureY ? "true" : "false";
	dimensionsElem->SetAttribute("repeatY",  repeatYAsStr);

	TiXmlElement * effectElement = element->FirstChildElement("effect");
	effectElement->SetDoubleAttribute("noiseintensity", mNoiseShaderIntensity);
	effectElement->SetDoubleAttribute("wobbleintensity", mWobbleShaderIntensity);
	effectElement->SetDoubleAttribute("pixelwobbleintensity", mPixelWobbleIntensity);
	effectElement->SetDoubleAttribute("pixelwobblespeedmod", mPixelWobbleSpeedMod);

	TiXmlElement * foliageSwayElement = new TiXmlElement("foliage_sway_props");

	foliageSwayElement->SetDoubleAttribute("bottom_left_intensity_x", mFoliageSwayProperties.BottomLeftSwayIntensity.X);
	foliageSwayElement->SetDoubleAttribute("bottom_left_intensity_y", mFoliageSwayProperties.BottomLeftSwayIntensity.Y);
	foliageSwayElement->SetDoubleAttribute("bottom_right_intensity_x", mFoliageSwayProperties.BottomRightSwayIntensity.X);
	foliageSwayElement->SetDoubleAttribute("bottom_right_intensity_y", mFoliageSwayProperties.BottomRightSwayIntensity.Y);
	foliageSwayElement->SetDoubleAttribute("top_left_intensity_x", mFoliageSwayProperties.TopLeftSwayIntensity.X);
	foliageSwayElement->SetDoubleAttribute("top_left_intensity_y", mFoliageSwayProperties.TopLeftSwayIntensity.Y);
	foliageSwayElement->SetDoubleAttribute("top_right_intensity_x", mFoliageSwayProperties.TopRightSwayIntensity.X);
	foliageSwayElement->SetDoubleAttribute("top_right_intensity_y", mFoliageSwayProperties.TopRightSwayIntensity.Y);
	foliageSwayElement->SetDoubleAttribute("time_multiplier", mFoliageSwayProperties.TimeMultiplier);

	element->LinkEndChild(foliageSwayElement);
}

void Sprite::SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[])
{
	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    
	if(mVertexBuffer)
	{
		mVertexBuffer->Release();
	}
	
	device->CreateBuffer( &bd, &InitData, &mVertexBuffer );
	GAME_ASSERT(mVertexBuffer);
}

void Sprite::SetVertexBufferBump(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormalTanBiNorm vertices[])
{
	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    
	if(mVertexBufferBump)
	{
		mVertexBufferBump->Release();
	}
	
	device->CreateBuffer( &bd, &InitData, &mVertexBufferBump );
	GAME_ASSERT(mVertexBufferBump);
}

void Sprite::SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[])
{	
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;        
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices;

	device->CreateBuffer( &bd, &InitData, &mIndexBuffer );
	GAME_ASSERT(mIndexBuffer);
}

void Sprite::Draw_effectBasic(ID3D10Device * graphicsdevice)
{
	//// set the world matrix
	m_effectBasic->SetWorld((float*)&m_world);

	// set the alpha value
	m_effectBasic->SetAlpha(m_alpha);

	//// Set the input layout on the device
	graphicsdevice->IASetInputLayout(m_effectBasic->InputLayout);

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	graphicsdevice->IASetVertexBuffers(0,1, &mVertexBuffer, &stride, &offset);

	//// set the index buffer
	graphicsdevice->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	graphicsdevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectBasic->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectBasic->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		graphicsdevice->DrawIndexed(4, 0 , 0);
	}
}

void Sprite::Draw_effectLightTexture(ID3D10Device * device)
{
	GAME_ASSERT(m_effectLightTexture);

	//// set the world matrix
 	m_effectLightTexture->SetWorld((float*)&m_world);

	// set the texture.
	m_effectLightTexture->SetTexture(m_texture);

	// set the alpha value
	m_effectLightTexture->SetAlpha(m_alpha);

	float timeDiff = Timing::Instance()->GetTotalTimeSeconds() - mBurstTintStartTime;
	if (timeDiff >= kBurstTintMaxTime)
	{
		mShowingBurstTint = false;
	}
	else
	{
		double gb_amount = (double)timeDiff / (double)kBurstTintMaxTime;
		m_effectLightTexture->SetLightColor((float*)D3DXVECTOR4(1, gb_amount, gb_amount, 1));
	}

	//// Set the input layout on the device
	device->IASetInputLayout(m_effectLightTexture->InputLayout);

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &mVertexBuffer, &stride, &offset);

	//// set the index buffer
	device->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectLightTexture->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectLightTexture->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0 , 0);
	}

	m_effectLightTexture->SetLightColor((float*)D3DXVECTOR4(1, 1, 1, 1));
}

void Sprite::Draw_effectPixelWobble(ID3D10Device * device)
{
	//// set the world matrix
	m_effectPixelWobble->SetWorld((float*)&m_world);

	// set the texture.
	m_effectPixelWobble->SetTexture(m_texture);

	// set the alpha value
	m_effectPixelWobble->SetAlpha(m_alpha);

	m_effectPixelWobble->SetWobbleIntensity(mPixelWobbleIntensity);

	m_effectPixelWobble->SetSpeedMod(mPixelWobbleSpeedMod);

	//// Set the input layout on the device
	device->IASetInputLayout(m_effectPixelWobble->InputLayout);

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &mVertexBuffer, &stride, &offset);

	//// set the index buffer
	device->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectPixelWobble->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectPixelWobble->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0 , 0);
	}
}

void Sprite::Draw_effectNoise(ID3D10Device * device)
{
	//// set the world matrix
	m_effectNoise->SetWorld((float*)&m_world);

	// set the texture.
	m_effectNoise->SetTexture(m_texture);

	// set the alpha value
	m_effectNoise->SetAlpha(m_alpha);

	m_effectNoise->SetNoiseAmount(mNoiseShaderIntensity);

	//// Set the input layout on the device
	device->IASetInputLayout(m_effectNoise->InputLayout);

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &mVertexBuffer, &stride, &offset);

	//// set the index buffer
	device->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectNoise->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectNoise->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0 , 0);
	}
}

void Sprite::Draw_effectBump(ID3D10Device * device)
{
	//// set the world matrix
	m_effectLightTextureBump->SetWorld((float*)&m_world);

	// set the texture.
	m_effectLightTextureBump->SetTexture(m_texture);

	m_effectLightTextureBump->SetBumpTexture(m_textureBump);

	// set the alpha value
	m_effectLightTextureBump->SetAlpha(m_alpha);

	// calculate light direction (camera is light position)
	Vector3 camPos = GameObjectManager::Instance()->GetPlayer()->Position();//Camera2D::GetInstance()->Position();
	Vector3 dir = camPos - m_position;
	dir.Z = -250; // TODO: scale based on frame buffer
	//dir.Y = 0;
	dir.Normalise();
	m_effectLightTextureBump->SetLightDirection((float*)D3DXVECTOR4(-dir.X, dir.Y , dir.Z, 1.0f));

	//// Set the input layout on the device
	device->IASetInputLayout(m_effectLightTextureBump->InputLayout);

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormalTanBiNorm);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &mVertexBufferBump, &stride, &offset);

	//// set the index buffer
	device->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectLightTextureBump->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectLightTextureBump->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0 , 0);
	}
}

void Sprite::Draw_effectLightTexWobble(ID3D10Device * device)
{
	//// set the world matrix
	m_effectLightTextureWobble->SetWorld((float*)&m_world);

	// set the texture. TODO: optimise this i.e not have to set the texture every time
	m_effectLightTextureWobble->SetTexture(m_texture);

	// set the alpha value on the shader
	m_effectLightTextureWobble->SetAlpha(m_alpha);

	// set the intensity of the wobble
	m_effectLightTextureWobble->SetWobbleIntensity(mWobbleShaderIntensity);

	//// Set the input layout on the device
	device->IASetInputLayout(m_effectLightTextureWobble->InputLayout);

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &mVertexBuffer, &stride, &offset);

	//// set the index buffer 
	device->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectLightTextureWobble->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectLightTextureWobble->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0 , 0);
	}
}

void Sprite::Draw_effectFoliageSway(ID3D10Device * device)
{
	//// set the world matrix
	m_effectFoliageSway->SetWorld((float*)&m_world);

	// set the texture. TODO: optimise this i.e not have to set the texture every time
	m_effectFoliageSway->SetTexture(m_texture);

	// set the alpha value on the shader
	m_effectFoliageSway->SetAlpha(m_alpha);

	m_effectFoliageSway->SetTimeMultiplier(mFoliageSwayProperties.TimeMultiplier);

	//// Set the input layout on the device
	device->IASetInputLayout(m_effectFoliageSway->InputLayout);

	// Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

	//// set the index buffer 
	device->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set primitive topology
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	D3D10_TECHNIQUE_DESC techDesc;
	m_effectFoliageSway->CurrentTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_effectFoliageSway->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->DrawIndexed(4, 0, 0);
	}
}

void Sprite::Draw_effectReflection(ID3D10Device * graphicsdevice)
{
}

void Sprite::UpdateAnimations()
{
	//NOTE: This is the default animation behaviour (just looks for "Body" and "Still" xml nodes) for a sprite
	// since we can't determine what a sprite wants to do we have to create separate objects
	// that inherit from Sprite and override their UpdateAnimations() function.
	AnimationPart * bodyPart = m_animation->GetPart("body");

	if(bodyPart != 0)
	{
		if(bodyPart->CurrentSequence()->Name() != "Still")
		{
			bodyPart->SetSequence("Still");
		}

		bodyPart->AnimateLooped();

		m_texture = bodyPart->CurrentFrame(); // set the current texture
	}
}

Animation * Sprite::GetAnimation()
{
	if (!m_isAnimated)
	{
		GAME_ASSERT(!m_isAnimated);
		return nullptr;
	}

	return m_animation;
}

void Sprite::CalculateTangentBinormal( VertexPositionTextureNormalTanBiNorm vertices[], Vector3 &tangent, Vector3 &binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;

	// Calculate the two vectors for this face.
	vector1[0] = vertices[1].Pos.x - vertices[0].Pos.x;
	vector1[1] = vertices[1].Pos.y - vertices[0].Pos.y;
	vector1[2] = vertices[1].Pos.z - vertices[0].Pos.z;

	vector2[0] = vertices[2].Pos.x - vertices[0].Pos.x;
	vector2[1] = vertices[2].Pos.y - vertices[0].Pos.y;
	vector2[2] = vertices[2].Pos.z - vertices[0].Pos.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertices[1].TexCoord.x - vertices[0].TexCoord.x;
	tvVector[0] = vertices[1].TexCoord.y - vertices[0].TexCoord.y;

	tuVector[1] = vertices[2].TexCoord.x - vertices[0].TexCoord.x;
	tvVector[1] = vertices[2].TexCoord.y - vertices[0].TexCoord.y;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.X = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.Y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.Z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.X = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.Y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.Z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.X * tangent.X) + (tangent.Y * tangent.Y) + (tangent.Z * tangent.Z));

	// Normalize the normal and then store it
	tangent.X = tangent.X / length;
	tangent.Y = tangent.Y / length;
	tangent.Z = tangent.Z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.X * binormal.X) + (binormal.Y * binormal.Y) + (binormal.Z * binormal.Z));
			
	// Normalize the normal and then store it
	binormal.X = binormal.X / length;
	binormal.Y = binormal.Y / length;
	binormal.Z = binormal.Z / length;
}

void Sprite::CalculateNormal(Vector3 tangent, Vector3 binormal, Vector3 &normal)
{
	float length;

	// Calculate the cross product of the tangent and binormal which will give the normal vector.
	normal.X = (tangent.Y * binormal.Z) - (tangent.Z * binormal.Y);
	normal.Y = (tangent.Z * binormal.X) - (tangent.X * binormal.Z);
	normal.Z = (tangent.X * binormal.Y) - (tangent.Y * binormal.X);

	// Calculate the length of the normal.
	length = sqrt((normal.X * normal.X) + (normal.Y * normal.Y) + (normal.Z * normal.Z));

	// Normalize the normal.
	normal.X = normal.X / length;
	normal.Y = normal.Y / length;
	normal.Z = normal.Z / length;
}

void Sprite::UpdateToParent()
{
	if (mAttachedTo)
	{
		GAME_ASSERT(dynamic_cast<Sprite*>(mAttachedTo.get()));

		Sprite * parentSprite = static_cast<Sprite *>(mAttachedTo.get());

		m_position.Y = mAttachedTo->Position().Y + mAttachedToOffset.Y;
		if (mUpdateToParentsOrientation)
		{
			if (parentSprite->IsHFlipped())
			{
				if (mParentHFlipInitial)
				{
					m_position.X = mAttachedTo->Position().X + mAttachedToOffset.X;
					if (mHflippedOnAttach)
					{
						FlipHorizontal();
					}
					else
					{
						UnFlipHorizontal();
					}
				}
				else
				{
					m_position.X = mAttachedTo->Position().X - mAttachedToOffset.X;
					if (mHflippedOnAttach)
					{
						UnFlipHorizontal();
					}
					else
					{
						FlipHorizontal();
					}
				}
			}
			else
			{
				if (mParentHFlipInitial)
				{
					m_position.X = mAttachedTo->Position().X - mAttachedToOffset.X;
					if (mHflippedOnAttach)
					{
						UnFlipHorizontal();
					}
					else
					{
						FlipHorizontal();
					}
				}
				else
				{
					m_position.X = mAttachedTo->Position().X + mAttachedToOffset.X;
					if (mHflippedOnAttach)
					{
						FlipHorizontal();
					}
					else
					{
						UnFlipHorizontal();
					}
				}
			}
		}
		else
		{
			m_position.X = mAttachedTo->Position().X + mAttachedToOffset.X;
		}
	}
}

void Sprite::AttachTo(std::shared_ptr<GameObject> & parent, Vector3 offset, bool trackParentsOrientation)
{
	DrawableObject::AttachTo(parent, offset, trackParentsOrientation);

	Sprite * sprite = dynamic_cast<Sprite *>(parent.get());
	GAME_ASSERT(sprite);
	if (!sprite)
	{
		return;
	}

	mParentHFlipInitial = sprite->IsHFlipped();
	mHflippedOnAttach = m_horizontalFlip;
}


