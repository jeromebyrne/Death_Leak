#ifndef HEALTHDEVIL_H
#define HEALTHDEVIL_H

#include "sprite.h"

class HealthDevil : public Sprite
{
public:

	HealthDevil(float x = 0.0f, 
		float y = 0.0f, 
		DepthLayer depthLayer = kPlayer, 
		float width = 10.0f, 
		float height = 10.0f,
		float groundFriction = 1.0f, 
		float airResistance = 1.0f);

	virtual ~HealthDevil(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

protected:

private:

	void GiveReward();

	void GiveHealthUpgradeReward();

	void GiveFocusUpgradeReward();

	bool mHasCreatedParticles = false;

	bool mHasPlayedDialog = false;

	ISound * mVoiceOverSoundPlaying = nullptr;

	bool mHasGivenReward = false;

};

#endif
