#ifndef DOOR_H
#define DOOR_H

#include "sprite.h"

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

	bool mIsInitiallyLocked = false;
	// this is to prevent the player entering the door immediately when we load the level
	float mDoorWarmUpTime = 0.0f;
	std::string mToLevelFile;
	Vector2 mToLevelPosition;
	std::string mRequiredKey;
	std::string mDoorOpenSFX;
	std::string mDoorLockedSFX;
	bool mCanTryOpen = true;
};

#endif
