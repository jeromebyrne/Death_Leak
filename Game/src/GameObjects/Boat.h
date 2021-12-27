#ifndef BOAT_H
#define BOAT_H

#include "sprite.h"
#include "Graphics.h"

class Boat : public Sprite
{
public:

	Boat();
	virtual ~Boat(void);

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	void EnterBoat();

	virtual void OnInteracted() override;
	virtual bool CanInteract() override;

	bool mIsInitiallyLocked = false;
	// this is to prevent the player entering the door immediately when we load the level
	float mBoatWarmUpTime = 0.0f;
	std::string mToLevelFile;
	Vector2 mToLevelPosition;
	std::string mBoatLockedSFX;
	bool mCanTryOpen = true;

	float mIsUnlockingCurrentTime = 0.0f;
};

#endif
