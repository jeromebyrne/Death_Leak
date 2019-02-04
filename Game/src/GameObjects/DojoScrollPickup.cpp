#include "precompiled.h"
#include "DojoScrollPickup.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "Game.h"

static const D3DXCOLOR kCostTextColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
static const float kCostYOffset = -105.0f;

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
}

void DojoScrollPickup::Initialise()
{
	Sprite::Initialise();

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0, 0);

	InitialiseCostText();
}

void DojoScrollPickup::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	string featureAsString = XmlUtilities::ReadAttributeAsString(element, "feature_unlock", "value");
	mUnlocksFeature = FeatureUnlockManager::GetInstance()->GetFeatureTypeFromString(featureAsString);

	mOrbCost = XmlUtilities::ReadAttributeAsInt(element, "orb_cost", "value");
	mCostOffsetX = XmlUtilities::ReadAttributeAsInt(element, "orb_cost", "offset_x");

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
	element->LinkEndChild(orbCostElement);
}

void DojoScrollPickup::OnInteracted()
{
	// TODO: remove the currency
	FeatureUnlockManager::GetInstance()->SetFeatureUnlocked(mUnlocksFeature);
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

void DojoScrollPickup::InitialiseCostText()
{
	// font setup
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
}

void DojoScrollPickup::Draw(ID3D10Device * device, Camera2D * camera)
{
	Sprite::Draw(device, camera);

	Vector2 worldPos = Vector2((m_position.X - (m_dimensions.X * 0.5f)) + mCostOffsetX, m_position.Y + kCostYOffset);
	Vector2 screenPos = Utilities::WorldToScreen(worldPos);

	RECT bounds = { screenPos.X, screenPos.Y, screenPos.X + m_dimensions.X, screenPos.Y + m_dimensions.Y };
	mCostText->DrawTextA(0, mOrbCostString.c_str(), -1, &bounds, DT_NOCLIP, kCostTextColor);
}

