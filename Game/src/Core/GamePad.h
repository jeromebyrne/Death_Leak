#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <XInput.h>
#pragma comment(lib, "XInput.lib")

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
