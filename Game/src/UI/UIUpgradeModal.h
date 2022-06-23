#ifndef UIUPGRADEMODAL_H
#define UIUPGRADEMODAL_H

#include "uiscreen.h"
#include "FeatureUnlockManager.h"

class UISprite;

class UIUpgradeModal : public UIScreen
{
public:

	UIUpgradeModal(string name);
	virtual ~UIUpgradeModal(void);

	virtual void Update() override;

	virtual void Initialise() override;

	virtual void LoadContent(ID3D10Device * graphicsdevice) override;

	virtual void Draw(ID3D10Device * device) override;

	void SetLocalizedDescription(const string & desc);

	void SetLocalizedTitle(const string & title);

	void SetUpgradeType(FeatureUnlockManager::FeatureType featureUnlock);

	void SetUpgradeCost(int cost);

private:

	bool CanPurchase();

	void DoPurchase();

	// string 
	string mLocalizedTitle;
	string mLocalizedDescription;
	ID3DX10Font* mDescriptionFont = nullptr;
	ID3DX10Font* mTitleFont = nullptr;
	ID3DX10Font* mCostFont = nullptr;

	float mDismissButtonCurrentWarmup = 0.0f;

	UISprite * mBackButtonSprite_gamepad = nullptr;
	UISprite * mBackButtonSprite_keyboard = nullptr;

	UISprite* mUpgradeButtonSprite_gamepad = nullptr;

	FeatureUnlockManager::FeatureType mFeatureType = FeatureUnlockManager::FeatureType::kNone;

	int mUpgradeCost = 0;
	string mUpgradeCostAsString;
};

#endif
