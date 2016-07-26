#ifndef INPUTMANAGER_H
#define ENVIRONMENT_H

class InputManager
{
public:

	InputManager();
	
	void ProcessGameplayInput();

	bool ShowDebugInfoEnabled() const { return mShowDebugInfo; }

	void DisableDebugInfoEnabled() { mShowDebugInfo = false; }

private:

	struct CurrentGameplayActions
	{
		bool mIsCrouching = false;
		bool mIsSprinting = false;
		Vector2 mAimDirection;
	};

	struct CurrentGamePadState
	{
		bool mPressingJump = false;
		bool mPressingWallJump = false;
		bool mPressingPrimaryWeapon = false;
		bool mPressingSecondaryWeapon = false;
		bool mPressingStrafeLeft = false;
		bool mPressingStrafeRight = false;
	};

	void ProcessGameplay_GamePad();

	void ProcessGameplay_Keyboard();

	void ProcessCrouch_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessLeftRightMovement_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessJump_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessWallJump_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessAimDirection_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessPrimaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessSecondaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessStrafing_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps);

	void ProcessTestActions_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps);

	bool mShowDebugInfo;
	bool mPressingDebugInfoKey;

	CurrentGamePadState mCurrentGamepadState;
};

#endif
