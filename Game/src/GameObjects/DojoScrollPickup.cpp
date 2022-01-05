#include "precompiled.h"
#include "DojoScrollPickup.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "Game.h"
#include "UIManager.h"
#include "StringManager.h"

static const D3DXCOLOR kCostTextColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
static const D3DXCOLOR kDescTextColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

static const float kCostYOffset = -111.0f;
static const float kCostYOffsetCliffhut = -100.0f;

DojoScrollPickup::DojoScrollPickup(float x,
									float y, 
									DepthLayer 
									depthLayer, 
									float width, 
									float height):
									Sprite(x,y, depthLayer, width, height)
{
}

DojoScrollPickup::~DojoScrollPickup(void)
{
	if (mCostText)
	{
		mCostText->Release();
		mCostText = nullptr;
	}

	if (mDescriptionText)
	{
		mDescriptionText->Release();
		mDescriptionText = nullptr;
	}
}

void DojoScrollPickup::Update(float delta)
{	
	Sprite::Update(delta);

#ifdef DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}
#endif

	if (FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(mUnlocksFeature))
	{
		GameObjectManager::Instance()->RemoveGameObject(this, true);
		return;
	}

	if (mHasAddedDescBacking == false)
	{
		AddDescriptionBacking();
		mHasAddedDescBacking = true;
	}

	m_alpha = CanInteract() ? 1.0f : 0.25f;

	if (mDescriptionBacking)
	{
		if (sCurrentInteractable == ID())
		{
			mDescriptionBacking->SetAlpha(1.0f);
		}
		else
		{
			mDescriptionBacking->SetAlpha(0.0f);
		}
	}
}

void DojoScrollPickup::Initialise()
{
	Sprite::Initialise();

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0.0f, 50.0f);

	InitialiseText();

	string featureAsString = FeatureUnlockManager::GetInstance()->GetFeatureAsString(mUnlocksFeature);
	string descKey = "dojo_scroll_desc_";
	descKey += featureAsString;
	mLocalizedDescription = StringManager::GetInstance()->GetLocalisedString(descKey.c_str());

	SetDepthLayer(GameObject::kGround);

	mCostOffsetY = kCostYOffset;

	string currentLevel = GameObjectManager::Instance()->GetCurrentLevelFile();
	if (currentLevel == "XmlFiles\\levels\\cliff_hut.xml")
	{
		mCostOffsetY = kCostYOffsetCliffhut;
	}
}

void DojoScrollPickup::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	string featureAsString = XmlUtilities::ReadAttributeAsString(element, "feature_unlock", "value");
	mUnlocksFeature = FeatureUnlockManager::GetInstance()->GetFeatureTypeFromString(featureAsString);

	mOrbCost = XmlUtilities::ReadAttributeAsInt(element, "orb_cost", "value");
	mCostOffsetX = XmlUtilities::ReadAttributeAsInt(element, "orb_cost", "offset_x");
	mDescriptionOffsetX = XmlUtilities::ReadAttributeAsFloat(element, "orb_cost", "desc_offset_x");

	mOrbCostString = std::to_string(mOrbCost);
}

void DojoScrollPickup::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	string featureAsString = FeatureUnlockManager::GetInstance()->GetFeatureAsString(mUnlocksFeature);

	TiXmlElement * unlockElement = new TiXmlElement("feature_unlock");
	unlockElement->SetAttribute("value", featureAsString.c_str());
	element->LinkEndChild(unlockElement);

	TiXmlElement * orbCostElement = new TiXmlElement("orb_cost");
	orbCostElement->SetAttribute("value", mOrbCost);
	orbCostElement->SetAttribute("offset_x", mCostOffsetX);
	orbCostElement->SetDoubleAttribute("desc_offset_x", mDescriptionOffsetX);
	element->LinkEndChild(orbCostElement);
}

void DojoScrollPickup::OnInteracted()
{
	if (mDescriptionBacking)
	{
		mDescriptionBacking->SetAlpha(0.0f);
	}

	int currentOrbCount = SaveManager::GetInstance()->GetNumCurrencyOrbsCollected();

	SaveManager::GetInstance()->SetNumCurrencyOrbsCollected(currentOrbCount - mOrbCost);
	
	FeatureUnlockManager::GetInstance()->SetFeatureUnlocked(mUnlocksFeature);

	// TODO: do some VFX and SFX
}

bool DojoScrollPickup::CanInteract()
{
	if (mUnlocksFeature == FeatureUnlockManager::kNone)
	{
		return false;
	}

	int currentOrbs = SaveManager::GetInstance()->GetNumCurrencyOrbsCollected();

	if (currentOrbs < mOrbCost)
	{
		return false;
	}

	return true;
}

void DojoScrollPickup::InitialiseText()
{
	// cost text
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 32;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, Utilities::ConvertCharStringToWcharString("Impact"));

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mCostText);
	}

	// description text
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 28;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, Utilities::ConvertCharStringToWcharString("Impact"));

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mDescriptionText);
	}
}

void DojoScrollPickup::Draw(ID3D10Device * device, Camera2D * camera)
{
	Sprite::Draw(device, camera);

	float worldScale = GameObjectManager::Instance()->GetCurrentLevelProperties().GetZoomInPercent();
	worldScale = (1.0f + (1.0f - worldScale));

	// cost text
	{
		Vector2 worldPos = Vector2((m_position.X - (m_dimensions.X * 0.5f)) + mCostOffsetX, m_position.Y + mCostOffsetY);

		worldPos = worldPos * worldScale;
		Vector2 screenPos = Utilities::WorldToScreen(worldPos);

		RECT bounds = { screenPos.X, screenPos.Y, screenPos.X + m_dimensions.X, screenPos.Y + m_dimensions.Y };
		mCostText->DrawTextA(0, mOrbCostString.c_str(), -1, &bounds, DT_NOCLIP, kCostTextColor);
	}
	
	// description text
	if (mDescriptionBacking && mDescriptionBacking->Alpha() > 0.0f)
	{
		Vector2 worldPos = Vector2((mDescriptionBacking->X() - (mDescriptionBacking->Dimensions().X * 0.5f)) + mCostOffsetX, mDescriptionBacking->Y() + 10.0f);
		worldPos = worldPos * worldScale;
		Vector2 screenPos = Utilities::WorldToScreen(worldPos);

		RECT bounds = { screenPos.X, screenPos.Y, screenPos.X + (mDescriptionBacking->Dimensions().X * 1.0f), screenPos.Y + (mDescriptionBacking->Dimensions().Y * 1.4f) };
		mCostText->DrawTextA(0, mLocalizedDescription.c_str(), -1, &bounds, DT_WORDBREAK, kDescTextColor);
	}
}

void DojoScrollPickup::AddDescriptionBacking()
{
	if (mDescriptionBacking)
	{
		return;
	}

	mDescriptionBacking = new Sprite();

	mDescriptionBacking->SetXY((m_position.X + 100.0f) + mDescriptionOffsetX, m_position.Y + 170.0f);

	mDescriptionBacking->SetIsNativeDimensions(true);

	mDescriptionBacking->SetTextureFilename("Media\\levels\\dojo\\desc_backing.png");

	mDescriptionBacking->EffectName = this->EffectName;

	mDescriptionBacking->SetDepthLayer(GameObject::kGroundBack);

	GameObjectManager::Instance()->AddGameObject(mDescriptionBacking);

	mDescriptionBacking->SetAlpha(0.5f);
}

