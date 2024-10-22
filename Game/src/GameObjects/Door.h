#ifndef DOOR_H
#define DOOR_H

#include "sprite.h"
#include "Graphics.h"

class Door : public Sprite
{
public:

	Door();
	virtual ~Door(void);

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	void EnterDoor();

	virtual void OnInteracted() override;
	virtual bool CanInteract() override;

	void DisplayDoorLockedEffect();

	bool mIsInitiallyLocked = false;
	// this is to prevent the player entering the door immediately when we load the level
	float mDoorWarmUpTime = 0.0f;
	std::string mToLevelFile;
	Vector2 mToLevelPosition;
	std::string mRequiredKey;
	std::string mDoorOpenSFX;
	std::string mDoorLockedSFX;
	bool mCanTryOpen = true;
	std::string mDoorIdentifier;

	float mIsUnlockingCurrentTime = 0.0f;
};

#endif
