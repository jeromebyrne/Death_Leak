#include "precompiled.h"
#include "GamePad.h"

GamePad * GamePad::mController1 = nullptr;

GamePad::GamePad(int playerNum)
{
	mControllerNum = playerNum -1;
}

GamePad::~GamePad(void)
{
}

XINPUT_STATE GamePad::GetState()
{
    // Zeroise the state
    ZeroMemory(&mControllerState, sizeof(XINPUT_STATE));

    // Get the state
    XInputGetState(mControllerNum, &mControllerState);

    return mControllerState;
}

bool GamePad::IsConnected()
{
    // Zeroise the state
    ZeroMemory(&mControllerState, sizeof(XINPUT_STATE));

    // Get the state
    DWORD Result = XInputGetState(mControllerNum, &mControllerState);

    if(Result == ERROR_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GamePad::Vibrate(int leftVal, int rightVal)
{
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;

    // Zeroise the Vibration
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    // Set the Vibration Values
    Vibration.wLeftMotorSpeed = leftVal;
    Vibration.wRightMotorSpeed = rightVal;

    // Vibrate the controller
    XInputSetState(mControllerNum, &Vibration);
}

GamePad * GamePad::GetPad1()
{
	if (!mController1)
	{
		mController1 = new GamePad(1);
	}
	return mController1;
}
