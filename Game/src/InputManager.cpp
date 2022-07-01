#include "precompiled.h"
#include "InputManager.h"
#include "GamePad.h"
#include "DXWindow.h"
#include "NinjaSpawner.h"
#include "SolidLineStrip.h"
#include "Projectile.h"
#include "Timing.h"
#include "UIManager.h"
#include "GhostEnemySpawner.h"
#include "NPCManager.h"

const float kMaxGamepadAnalogRange = (std::numeric_limits<short>::max)();
const float kAimOffsetX = 450.0f;
const float kAimOffsetY = 300.0f;
const float kJumpInitialPercent = 40.0f;
const float kJumpIncrementalIncreasePercent = 6.5f;
const float kMaxJumpPercent = 115.0f;
const int kMaxVibrationValue = 65535;

InputManager::InputManager() :
	mShowDebugInfo(false),
	mPressingDebugInfoKey(false),
	mLastTimePressedRoll(999.0f),
	mLastTimePressedJump(999.0f),
	mEnableGraphicsPostProcessing(true),
	mPressingPostProcessingKey(false)
{
}

void InputManager::Update(float delta)
{
	if (m_CurrentTimeVibrating > 0.0f)
	{
		m_CurrentTimeVibrating -= delta;

		if (m_CurrentTimeVibrating < 0.0f)
		{
			Vibrate(0.0f, 0.0f, 0.0f);
		}
	}
}

void InputManager::Vibrate(float leftPercent, float rightPercent, float time)
{
	GamePad * gamepad1 = GamePad::GetPad1();

	if (gamepad1 == nullptr || !gamepad1->IsConnected())
	{
		return;
	}

	m_CurrentTimeVibrating = time;

	if (leftPercent > 1.0f)
	{
		leftPercent = 1.0f;
	}

	if (rightPercent > 1.0f)
	{
		rightPercent = 1.0f;
	}

	gamepad1->Vibrate(kMaxVibrationValue * leftPercent, kMaxVibrationValue * rightPercent);
}

void InputManager::ProcessGameplayInput()
{
	/*
	if (GetForegroundWindow() != DXWindow::GetInstance()->Hwnd())
	{
		return;
	}
	*/

#ifdef _DEBUG

	if (GetAsyncKeyState('I'))
	{
		if (!UIManager::Instance()->IsObjectEditorDisplaying())
		{
			mPressingDebugInfoKey = true;
		}
	}
	else
	{
		if (mPressingDebugInfoKey) // just released
		{
			if (mShowDebugInfo)
			{
				mShowDebugInfo = false;
			}
			else
			{
				mShowDebugInfo = true;
			}
		}
		mPressingDebugInfoKey = false;
	}

	if (GetAsyncKeyState('L'))
	{
		if (!UIManager::Instance()->IsObjectEditorDisplaying())
		{
			mPressingPostProcessingKey = true;
		}
	}
	else
	{
		if (mPressingPostProcessingKey)
		{
			if (mEnableGraphicsPostProcessing)
			{
				mEnableGraphicsPostProcessing = false;
			}
			else
			{
				mEnableGraphicsPostProcessing = true;
			}
		}
		mPressingPostProcessingKey = false;
	}

#endif
	
	Player * player = GameObjectManager::Instance()->GetPlayer();

	if (!player || player->IsDead() || !player->CanBeControlled())
	{
		return;
	}

	GamePad * gamepad1 = GamePad::GetPad1();

	if (gamepad1 && gamepad1->IsConnected())
	{
		ProcessGameplay_GamePad(player);
	}
	else
	{
		ProcessGameplay_Keyboard(player);
	}
}

void InputManager::ProcessCrouch_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (padState.Gamepad.sThumbLY < -(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * 2.5f) &&
		!player->IsDoingMelee() &&
		player->IsOnSolidSurface() &&
		/*!player->IsStrafing() &&*/
		!player->GetIsCollidingAtObjectSide() &&
		!player->GetIsRolling())
	{
		currentActions.mIsCrouching = true;
		player->StopXAccelerating();
	}

	player->SetCrouching(currentActions.mIsCrouching);
}

void InputManager::ProcessLeftRightMovement_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (player->JustFellFromLargeDistance() ||
		player->JustFellFromShortDistance() ||
		currentActions.mIsCrouching ||
		player->GetIsRolling())
	{
		return;
	}

	float range = GetThumbstickRange(padState.Gamepad.sThumbLX);
	float absRange = std::abs(range);

	if (!player->IsDoingMelee() &&
		!player->IsWallJumping() &&
		absRange > 0.2f)
	{
		player->AccelerateX(range * 100.0f/*, absRange */);
	}
	else
	{
		player->StopXAccelerating();
	}
}

void InputManager::ProcessJump_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (player->GetIsDownwardDashing())
	{
		// prioritise downward dash over double jumping
		return;
	}

	bool wasPressingJump = mCurrentGamepadState.mPressingJump;

	// TODO: why am I doing all this before even checking if jumping?
	static float currentJumpIncreasePercent = kJumpInitialPercent;

	if (player->IsOnSolidSurface() || player->GetIsInWater())
	{
		// reset
		currentJumpIncreasePercent = kJumpInitialPercent;
	}

	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->IsDoingMelee() &&
		(/* padState.Gamepad.wButtons & XINPUT_GAMEPAD_A || */
		padState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD))
	{
		mCurrentGamepadState.mPressingJump = true;
	}
	else
	{
		mCurrentGamepadState.mPressingJump = false;
	}

	if (mCurrentGamepadState.mPressingJump && !wasPressingJump)
	{
		/*
		if (padState.Gamepad.sThumbLY < -30000 &&
			player->IsCrouching() && 
			!player->IsDoingMelee() &&
			player->IsOnSolidLine() &&
			player->GetCurrentSolidLineStrip() &&
			player->GetCurrentSolidLineStrip()->GetCanDropDown())
		{
			player->DropDown(); 
		}
		else
		{*/
			mLastTimePressedJump = Timing::Instance()->GetTotalTimeSeconds();
			float jumpPower = kJumpInitialPercent;

			player->Jump(jumpPower);
		//}
	}
	else if (mCurrentGamepadState.mPressingJump && 
		wasPressingJump && 
		player->CanIncreaseJumpIntensity() && 
		currentJumpIncreasePercent < kMaxJumpPercent)
	{
		currentJumpIncreasePercent += kJumpIncrementalIncreasePercent;
		player->IncreaseJump(currentJumpIncreasePercent);
	}
}

void InputManager::ProcessSwim_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps)
{
	if (!player->WasInWaterLastFrame())
	{
		return;
	}
	
	if (padState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		if (player->IsOnSolidSurface())
		{
			player->WaterJump();
		}
		player->AccelerateY(1.0f, 0.3f);
	}
	else if (padState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		player->AccelerateY(-1.0f, 0.3f);
	}
}

void InputManager::ProcessDownwardDash_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (player->GetIsDownwardDashing())
	{
		return;
	}

	bool wasPressingDownwardDash = mCurrentGamepadState.mPressingDownwardDashPrimary;

	if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER /*&&
		padState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * 2.5f*/)
	{
		mCurrentGamepadState.mPressingDownwardDashPrimary = true;
	}
	else
	{
		mCurrentGamepadState.mPressingDownwardDashPrimary = false;
	}

	if (mCurrentGamepadState.mPressingDownwardDashPrimary &&
		!wasPressingDownwardDash)
	{
		player->DoDownwardDash();
	}
}

void InputManager::ProcessSlowMotion_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (padState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		player->TryFocus();
	}
	else
	{
		// don't stop focusing if dead, as we should always be in slo mo when dead
		if (!player->IsDead())
		{
			player->StopFocus();
		}
	}
}

void InputManager::ProcessRoll_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	bool wasPressingRoll = mCurrentGamepadState.mPressingRoll;

	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->IsDoingMelee() &&
		(/*padState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||*/
			padState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER))
	{
		mCurrentGamepadState.mPressingRoll = true;
	}
	else
	{
		mCurrentGamepadState.mPressingRoll = false;
	}

	if (mCurrentGamepadState.mPressingRoll && 
		!wasPressingRoll &&
		!player->GetIsRolling())
	{ 
		mLastTimePressedRoll = Timing::Instance()->GetTotalTimeSeconds();
		player->Roll();
	}
}

void InputManager::ProcessSprint_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	/*
	if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
	{
		player->TrySprint();
	}
	else
	{
		player->StopSprint();
	}
	*/
}

void InputManager::ProcessMelee_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	bool wasPressingMelee = mCurrentGamepadState.mPressingMelee;

	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->IsDoingMelee() &&
		padState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
	{
		mCurrentGamepadState.mPressingMelee = true;
	}
	else
	{
		mCurrentGamepadState.mPressingMelee = false;
	}

	if (mCurrentGamepadState.mPressingMelee && !wasPressingMelee)
	{
		player->DoMeleeAttack();
	}
}

void InputManager::ProcessAimDirection_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps)
{
	currentActions.mAimDirection = Vector2(player->IsStrafing() ? player->GetStrafeDirectionX() : player->DirectionX(), 0.0f);
	Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();

	if (std::abs(padState.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		std::abs(padState.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{ 
		currentActions.mAimDirection = Vector2(padState.Gamepad.sThumbRX, padState.Gamepad.sThumbRY);
		currentActions.mAimDirection.Normalise();
	}
	else
	{
		Camera2D::GetInstance()->SetTargetOffsetY(defaultOffset.Y);
		return;
	}

	float range = GetThumbstickRange(padState.Gamepad.sThumbRY);

	Camera2D::GetInstance()->SetTargetOffsetY(defaultOffset.Y + (kAimOffsetY * range));

	player->SetAimLineDirection(currentActions.mAimDirection);
}

void InputManager::ProcessPrimaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->IsDoingMelee() &&
		(std::abs(padState.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
			std::abs(padState.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
	{
		mCurrentGamepadState.mPressingPrimaryWeapon = true;

		// let the player fire and return a projectile object which is added to the world
		Projectile * p = player->FireWeapon(currentActions.mAimDirection, 1.0f);

		if (p)
		{
			GameObjectManager::Instance()->AddGameObject(p);

			Vibrate(0.0f, 0.08f, 0.06f);
		}
	}
	else
	{
		mCurrentGamepadState.mPressingPrimaryWeapon = false;
	}
}

void InputManager::ProcessSecondaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->GetIsCollidingAtObjectSide() &&
		!player->IsDoingMelee() &&
		padState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
	{
		mCurrentGamepadState.mPressingSecondaryWeapon = true;
	}
	else
	{
		if (mCurrentGamepadState.mPressingSecondaryWeapon)
		{
			Projectile * p = player->FireBomb(currentActions.mAimDirection);

			if (p)
			{
				GameObjectManager::Instance()->AddGameObject(p);
			}
		}
		mCurrentGamepadState.mPressingSecondaryWeapon = false;
	}
}

void InputManager::ProcessStrafing_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps)
{
	// false by default
	player->SetIsStrafing(false);

	if (!(std::abs(padState.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
		!(std::abs(padState.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))

	{
		Camera2D::GetInstance()->SetTargetOffset(levelProps.GetOriginalTargetOffset());
		return;
	}

	if (player->GetIsRolling())
	{
		return;
	}

	if (player->JustFellFromLargeDistance() ||
		player->JustFellFromShortDistance() ||
		player->IsDoingMelee())
	{
		return;
	}	

	float rightThumbstickRangeX = GetThumbstickRange(padState.Gamepad.sThumbRX);

	bool isStrafing = false;
	
	if (rightThumbstickRangeX <= 0.0f && padState.Gamepad.sThumbLX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		isStrafing = true;
		player->SetStrafeDirectionX(-1.0f);
	}
	else if (rightThumbstickRangeX > 0.0f && padState.Gamepad.sThumbLX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		isStrafing = true;
		player->SetStrafeDirectionX(1.0f);
	}
	else if (rightThumbstickRangeX > 0.0f)
	{
		// firing but not moving
		player->UnFlipHorizontal();
	}
	else if (rightThumbstickRangeX < 0.0f)
	{
		// firing but not moving
		player->FlipHorizontal();
	}

	player->SetIsStrafing(isStrafing);

	Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();
	Camera2D::GetInstance()->SetTargetOffsetX(defaultOffset.X + (kAimOffsetX * rightThumbstickRangeX));
}

void InputManager::ProcessTestActions_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps)
{
	// Ninja spawning
	{
		static bool pressingLeftShoulder = false;
		if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_B)
		{
			pressingLeftShoulder = true;
		}
		else
		{
			if (pressingLeftShoulder)
			{
				// testing
				// NinjaSpawner spawner;
				// spawner.SpawnMultiple(3, Vector2(player->X(), player->Y()), Vector2(1200, 1200));

				// GhostEnemySpawner ghostSpawner;
				// ghostSpawner.SpawnMultiple(3, Vector2(player->X(), player->Y()), Vector2(1200, 1200));
			}

			pressingLeftShoulder = false;
		}
	}

	// slow motion
	{
		static bool pressing_slo_mo = false;
		if (GetAsyncKeyState('O'))
		{
			pressing_slo_mo = true;
		}
		else
		{
			if (pressing_slo_mo)
			{
				LOG_INFO("This is a really bad way to do this. Come back later.");
				if (Timing::Instance()->GetTimeModifier() == 1.0f)
				{
					Timing::Instance()->SetTimeModifier(0.2f);
				}
				else
				{
					Timing::Instance()->SetTimeModifier(1.0f);
				}
			}
			pressing_slo_mo = false;
		}
	}
}

void InputManager::ProcessGameplay_GamePad(Player * player)
{
	GamePad * gamepad1 = GamePad::GetPad1();

	XINPUT_STATE padState = gamepad1->GetState();

	const auto & levelProps = GameObjectManager::Instance()->GetCurrentLevelProperties();

	CurrentGameplayActions currentActions;

	ProcessSlowMotion_gamepad(padState, currentActions, player);

	ProcessDownwardDash_gamepad(padState, currentActions, player);

	ProcessMelee_gamepad(padState, currentActions, player);

	ProcessCrouch_gamepad(padState, currentActions, player);

	ProcessRoll_gamepad(padState, currentActions, player);

	ProcessSprint_gamepad(padState, currentActions, player);

	ProcessLeftRightMovement_gamepad(padState, currentActions, player);

	ProcessJump_gamepad(padState, currentActions, player);

	ProcessAimDirection_gamepad(padState, currentActions, player, levelProps);

	ProcessPrimaryWeapon_gamepad(padState, currentActions, player);

	// rmeoving bombs for now
	// ProcessSecondaryWeapon_gamepad(padState, currentActions, player);

	ProcessStrafing_gamepad(padState, currentActions, player, levelProps);

	ProcessSwim_gamepad(padState, currentActions, player, levelProps);

	ProcessTestActions_gamepad(padState, currentActions, player, levelProps);
}

void InputManager::ProcessGameplay_Keyboard(Player * player)
{
#ifndef _DEBUG
	return;
#endif

	const auto & levelProps = GameObjectManager::Instance()->GetCurrentLevelProperties();

	CurrentGameplayActions currentActions;

	ProcessLeftRightMovement_keyboard(currentActions, player);

	ProcessCrouch_keyboard(currentActions, player);
	
	ProcessJump_keyboard(currentActions, player);

	ProcessRoll_keyboard(currentActions, player);
}

void InputManager::ProcessLeftRightMovement_keyboard(CurrentGameplayActions & currentActions, Player * player)
{
	if (GameObjectManager::Instance()->GetCurrentLevelProperties().IsAnimationPreview())
	{
		return;
	}

	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() && 
		!currentActions.mIsCrouching &&
		!player->GetIsRolling())
	{
		if (GetAsyncKeyState('A') < 0 &&
			!player->IsDoingMelee() &&
			!player->IsWallJumping())
		{
			player->AccelerateX(-100.0f);
		}
		else if (GetAsyncKeyState('D') < 0 &&
			!player->IsDoingMelee() &&
			!player->IsWallJumping())
		{
			player->AccelerateX(100.0f);
		}
		else
		{
			// not pressing anything
			player->StopXAccelerating();
		}
	}

	player->SetSprintActive(currentActions.mIsSprinting);
}

void InputManager::ProcessCrouch_keyboard(CurrentGameplayActions & currentActions, Player * player)
{
	if (GetAsyncKeyState('S') < 0 &&
		!player->IsDoingMelee() &&
		player->IsOnSolidSurface() &&
		/*!player->IsStrafing() &&*/
		!player->GetIsCollidingAtObjectSide() &&
		!player->GetIsRolling())
	{
		currentActions.mIsCrouching = true;
		player->StopXAccelerating();
	}

	player->SetCrouching(currentActions.mIsCrouching);
}

void InputManager::ProcessJump_keyboard(CurrentGameplayActions & currentActions, Player * player)
{
	if (player->GetIsDownwardDashing())
	{
		// prioritise downward dash over double jumping
		return;
	}

	bool wasPressingJump = mCurrentGamepadState.mPressingJump;

	// TODO: why am I doing all this before even checking if jumping?
	static float currentJumpIncreasePercent = kJumpInitialPercent;

	if (player->IsOnSolidSurface() || player->GetIsInWater())
	{
		// reset
		currentJumpIncreasePercent = kJumpInitialPercent;
	}

	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->IsDoingMelee() &&
		GetAsyncKeyState('W') < 0)
	{
		mCurrentGamepadState.mPressingJump = true;
	}
	else
	{
		mCurrentGamepadState.mPressingJump = false;
	}

	if (mCurrentGamepadState.mPressingJump && !wasPressingJump)
	{
		/*
		if (player->IsCrouching() &&
			!player->IsDoingMelee() &&
			player->IsOnSolidLine() &&
			player->GetCurrentSolidLineStrip() &&
			player->GetCurrentSolidLineStrip()->GetCanDropDown())
		{
			player->DropDown();
		}
		else 
		{
		*/
			mLastTimePressedJump = Timing::Instance()->GetTotalTimeSeconds();
			float jumpPower = kJumpInitialPercent;

			player->Jump(jumpPower);
		// }
	}
	else if (mCurrentGamepadState.mPressingJump &&
		wasPressingJump &&
		player->CanIncreaseJumpIntensity() &&
		currentJumpIncreasePercent < 100.0f)
	{
		currentJumpIncreasePercent += kJumpIncrementalIncreasePercent;
		player->IncreaseJump(currentJumpIncreasePercent);
	}
}

void InputManager::ProcessRoll_keyboard(CurrentGameplayActions & currentActions, Player * player)
{
	bool wasPressingRoll = mCurrentGamepadState.mPressingRoll;

	if (!player->JustFellFromLargeDistance() &&
		!player->JustFellFromShortDistance() &&
		!player->IsDoingMelee() &&
		GetAsyncKeyState(VK_SPACE) < 0)
	{
		mCurrentGamepadState.mPressingRoll = true;
	}
	else
	{
		mCurrentGamepadState.mPressingRoll = false;
	}

	if (mCurrentGamepadState.mPressingRoll &&
		!wasPressingRoll &&
		!player->GetIsRolling())
	{
		mLastTimePressedRoll = Timing::Instance()->GetTotalTimeSeconds();
		player->Roll();
	}
}

float InputManager::GetThumbstickRange(short thumbstickValue)
{
	return (float)thumbstickValue / kMaxGamepadAnalogRange;
}

float InputManager::GetTriggerRange(short triggerValue)
{
	if (triggerValue > 255)
	{
		return 1.0f;
	}
	return (float)triggerValue / 255.0f;
}

bool InputManager::IsPressingInteractButton() const
{
	GamePad * gamepad1 = GamePad::GetPad1();

	if (gamepad1 && gamepad1->IsConnected())
	{
		XINPUT_STATE padState = gamepad1->GetState();

		if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_X)
		{
			return true;
		}

		return false;
	}
	else
	{
		if (GetAsyncKeyState('E') < 0)
		{
			return true;
		}

		return false;
	}
}