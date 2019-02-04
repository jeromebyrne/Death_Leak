#ifndef DOJOSCROLLPICKUP_H
#define DOJOSCROLLPICKUP_H

#include "sprite.h"
#include "FeatureUnlockManager.h"

class DojoScrollPickup : public Sprite
{
public:

	DojoScrollPickup(float x = 0.0f,
		float y = 0.0f, 
		DepthLayer depthLayer = kPlayer, 
		float width = 10.0f, 
		float height = 10.0f);

	virtual ~DojoScrollPickup(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;

protected:

	virtual void OnInteracted() override;
	virtual bool CanInteract() override;

private:

	void InitialiseCostText();

	FeatureUnlockManager::FeatureType mUnlocksFeature = FeatureUnlockManager::kNone;

	int mOrbCost = 9999;

	ID3DX10Font* mCostText = nullptr;
	string mOrbCostString;
	float mCostOffsetX = 0.0f;
};

#endif
