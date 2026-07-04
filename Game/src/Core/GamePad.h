#ifndef GAMEPAD_H
#define GAMEPAD_H

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
#include "Engine/Platform/MacDxCompat.h"
#else
#include <XInput.h>
#pragma comment(lib, "XInput.lib")
#endif

class GamePad
{
public:

	GamePad(int playerNum);
	~GamePad(void);

	XINPUT_STATE GetState();
    bool IsConnected();
    void Vibrate(int leftVal = 0, int rightVal = 0);

	static GamePad * GetPad1();

private:

	static GamePad * mController1;

	XINPUT_STATE mControllerState;
	int mControllerNum;
};

#endif
