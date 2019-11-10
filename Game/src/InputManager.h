#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

class InputManager
{
public:

	InputManager();
	
	void ProcessGameplayInput();

	bool ShowDebugInfoEnabled() const { return mShowDebugInfo; }

	void EnableDebugInfo(bool enabled) { mShowDebugInfo = enabled; }

	void EnablePostProcessing(bool enabled) { mEnableGraphicsPostProcessing = enabled; }

	bool GraphicsPostProcessingEnabled() const { return mEnableGraphicsPostProcessing; }

	float GetLastTimePressedRoll() const { return mLastTimePressedRoll; }

	float GetLastTimePressedJump() const { return mLastTimePressedJump; }

	static float GetThumbstickRange(short thumbstickValue);

	static float GetTriggerRange(short triggerValue);

	bool IsPressingInteractButton() const;

	void Vibrate(float leftPercent, float rightPercent, float timeSeconds);

	void Update(float delta);

private:

	struct CurrentGameplayActions
	{
		bool mIsCrouching = false;
		bool mIsSprinting = false;
		Vector2 mAimDirection;
	};

	struct CurrentGamePadState
	{ 
		bool mPressingRoll = false;
		bool mPressingJump = false;
		bool mPressingWallJump = false;
		bool mPressingPrimaryWeapon = false;
		bool mPressingSecondaryWeapon = false;
		bool mPressingStrafeLeft = false;
		bool mPressingStrafeRight = false;
		bool mPressingMelee = false;
		bool mPressingDownwardDashPrimary = false;
	};

	void ProcessGameplay_GamePad(Player * player);

	void ProcessGameplay_Keyboard(Player * player);

	void ProcessCrouch_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessLeftRightMovement_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessJump_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessRoll_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessSprint_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessMelee_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessAimDirection_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps);

	void ProcessPrimaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessSecondaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessStrafing_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps);

	void ProcessSwim_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps);

	void ProcessDownwardDash_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	void ProcessTestActions_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps);

	void ProcessSlowMotion_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player);

	// keyboard
	void ProcessLeftRightMovement_keyboard(CurrentGameplayActions & currentActions, Player * player);

	void ProcessCrouch_keyboard(CurrentGameplayActions & currentActions, Player * player);

	void ProcessJump_keyboard(CurrentGameplayActions & currentActions, Player * player);

	void ProcessRoll_keyboard(CurrentGameplayActions & currentActions, Player * player);

	bool mShowDebugInfo;
	bool mPressingDebugInfoKey;
	bool mEnableGraphicsPostProcessing;
	bool mPressingPostProcessingKey;

	CurrentGamePadState mCurrentGamepadState;

	float mLastTimePressedRoll;
	float mLastTimePressedJump;

	float m_CurrentTimeVibrating = 0.0f;
};

#endif
