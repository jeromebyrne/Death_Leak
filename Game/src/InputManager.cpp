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

InputManager::InputManager() :
	mShowDebugInfo(false),
	mPressingDebugInfoKey(false),
	mLastTimePressedRoll(999.0f),
	mLastTimePressedJump(999.0f),
	mEnableGraphicsPostProcessing(true),
	mPressingPostProcessingKey(false)
{
}

void InputManager::ProcessGameplayInput()
{
	if (GetForegroundWindow() != DXWindow::GetInstance()->Hwnd())
	{
		return;
	}

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

	GamePad * gamepad1 = GamePad::GetPad1();

	if (gamepad1 && gamepad1->IsConnected())
	{
		ProcessGameplay_GamePad();
	}
	else
	{
		ProcessGameplay_Keyboard();
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
	if (!player->JustFellFromLargeDistance() &&
		!currentActions.mIsCrouching &&
		!player->GetIsRolling())
	{
		if (padState.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && 
			!player->IsDoingMelee() &&
			!player->IsWallJumping())
		{
			player->AccelerateX(-100);
			currentActions.mIsSprinting = true;
		}
		else if (padState.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			!player->IsDoingMelee() &&
			!player->IsWallJumping())
		{
			player->AccelerateX(100);
			currentActions.mIsSprinting = true;
		}
		else
		{
			// not pressing anything
			player->StopXAccelerating();
		}
	}

	player->SetSprintActive(currentActions.mIsSprinting);
}

void InputManager::ProcessJump_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (player->GetIsDownwardDashing())
	{
		// prioritise downward dash over double jumping
		return;
	}

	bool wasPressingJump = mCurrentGamepadState.mPressingJump;

	float initialJumpPercent = 70.0f;
	static float currentJumpIncreasePercent = initialJumpPercent;

	if (player->IsOnSolidSurface())
	{
		currentJumpIncreasePercent = initialJumpPercent;
	}

	if (!player->JustFellFromLargeDistance() &&
		!player->IsDoingMelee() &&
		(padState.Gamepad.wButtons & XINPUT_GAMEPAD_A || 
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
		if (padState.Gamepad.sThumbLY < -30000 &&
			!player->IsDoingMelee() &&
			player->IsOnSolidLine() &&
			player->GetCurrentSolidLineStrip() &&
			player->GetCurrentSolidLineStrip()->GetCanDropDown())
		{
			player->DropDown(); 
		}
		else
		{
			mLastTimePressedJump = Timing::Instance()->GetTotalTimeSeconds();
			float jumpPower = /* player->IsStrafing() ? initialJumpPercent * 0.8f :*/ initialJumpPercent;
			player->Jump(jumpPower);
		}
	}
	else if (mCurrentGamepadState.mPressingJump && wasPressingJump && player->CanIncreaseJumpIntensity() && currentJumpIncreasePercent < 100.0f)
	{
		currentJumpIncreasePercent += 7.0f;
		player->IncreaseJump(currentJumpIncreasePercent);
	}
}

void InputManager::ProcessSwimDown_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps)
{
	if (!(player->WasInWaterLastFrame() && player->GetWaterIsDeep()))
	{
		return;
	}

	if (player->IsOnSolidSurface())
	{
		return;
	}

	if (padState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		player->AccelerateY(-1.0f, 0.1f);
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
		player->StopFocus();
	}
}

void InputManager::ProcessRoll_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	bool wasPressingRoll = mCurrentGamepadState.mPressingRoll;

	if (!player->JustFellFromLargeDistance() &&
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

void InputManager::ProcessMelee_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	bool wasPressingMelee = mCurrentGamepadState.mPressingMelee;

	if (!player->JustFellFromLargeDistance() &&
		!player->IsDoingMelee() &&
		(padState.Gamepad.wButtons & XINPUT_GAMEPAD_X ||
		padState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER))
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

void InputManager::ProcessWallJump_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	bool wasPressingJump = mCurrentGamepadState.mPressingWallJump;

	if (!player->JustFellFromLargeDistance() &&
		padState.Gamepad.wButtons & XINPUT_GAMEPAD_A)
	{
		mCurrentGamepadState.mPressingWallJump = true;
	}
	else
	{
		mCurrentGamepadState.mPressingWallJump = false;
	}

	if (mCurrentGamepadState.mPressingWallJump && !wasPressingJump)
	{
		if (player->GetIsCollidingAtObjectSide())
		{
			player->WallJump(-player->DirectionX(), 100.0f);
		}
	}
}

void InputManager::ProcessAimDirection_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player, const LevelProperties & levelProps)
{
	currentActions.mAimDirection = Vector2(player->IsStrafing() ? player->GetStrafeDirectionX() : player->DirectionX(), 0);

	/*if (std::abs(padState.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ||
		std::abs(padState.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
	{*/
	currentActions.mAimDirection = Vector2(padState.Gamepad.sThumbRX, padState.Gamepad.sThumbRY);
	currentActions.mAimDirection.Normalise();
	//}

	Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();

	if (std::abs(currentActions.mAimDirection.Y) > std::abs(currentActions.mAimDirection.X))
	{
		// aiming up or down
		float yOffset = 150.0f;
		Camera2D::GetInstance()->SetTargetOffsetY(currentActions.mAimDirection.Y > 0.0f ? defaultOffset.Y + yOffset : defaultOffset.Y - yOffset);
	}
	else
	{
		Camera2D::GetInstance()->SetTargetOffsetY(defaultOffset.Y);
	}

	player->SetAimLineDirection(currentActions.mAimDirection);
}

void InputManager::ProcessPrimaryWeapon_gamepad(XINPUT_STATE padState, CurrentGameplayActions & currentActions, Player * player)
{
	if (!player->JustFellFromLargeDistance() &&
		!player->IsDoingMelee() &&
		(std::abs(padState.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
			std::abs(padState.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
	{
		mCurrentGamepadState.mPressingPrimaryWeapon = true;

		// let the player fire and return a projectile object which is added to the world
		Projectile * p = player->FireWeapon(currentActions.mAimDirection);

		if (p)
		{
			GameObjectManager::Instance()->AddGameObject(p);
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

	if (player->GetIsRolling())
	{
		return;
	}


	if (player->JustFellFromLargeDistance() ||
		player->IsDoingMelee())
	{
		return;
	}

	if (!(std::abs(padState.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
		!(std::abs(padState.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))

	{
		Camera2D::GetInstance()->SetTargetOffset(levelProps.GetOriginalTargetOffset());
		Camera2D::GetInstance()->SetOverrideDirection(false, Vector2(0, 0));
		
		return;
	}

	float camXOffset = 250.0f;

	if (padState.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		player->DirectionX() < 0)
	{
		player->SetIsStrafing(true);
		player->SetStrafeDirectionX(1.0f);
		Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();
		Camera2D::GetInstance()->SetTargetOffsetX(defaultOffset.X + camXOffset);
		Camera2D::GetInstance()->SetOverrideDirection(true, 1.0f);
		player->GetStrafeDirectionX() > 0.0f ? player->UnFlipHorizontal() : player->FlipHorizontal();
	}
	else if (padState.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		player->DirectionX() > 0)
	{
		Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();
		Camera2D::GetInstance()->SetTargetOffsetX(defaultOffset.X + camXOffset);
	}
	else if (padState.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		player->DirectionX() > 0)
	{
		player->SetIsStrafing(true);
		player->SetStrafeDirectionX(-1.0f);
		Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();
		Camera2D::GetInstance()->SetTargetOffsetX(defaultOffset.X + camXOffset);
		Camera2D::GetInstance()->SetOverrideDirection(true, -1.0f);
		player->GetStrafeDirectionX() > 0.0f ? player->UnFlipHorizontal() : player->FlipHorizontal();
	}
	else if (padState.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		player->DirectionX() < 0)
	{
		Vector2 defaultOffset = levelProps.GetOriginalTargetOffset();
		Camera2D::GetInstance()->SetTargetOffsetX(defaultOffset.X + camXOffset);
	}
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
				NinjaSpawner spawner;
				spawner.SpawnMultiple(3, Vector2(player->X(), player->Y()), Vector2(1200, 1200));

				GhostEnemySpawner ghostSpawner;
				ghostSpawner.SpawnMultiple(3, Vector2(player->X(), player->Y()), Vector2(1200, 1200));
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

void InputManager::ProcessGameplay_GamePad()
{
	GamePad * gamepad1 = GamePad::GetPad1();

	XINPUT_STATE padState = gamepad1->GetState();

	Player * player = GameObjectManager::Instance()->GetPlayer();

	if (!player)
	{
		return;
	}

	const auto & levelProps = GameObjectManager::Instance()->GetCurrentLevelProperties();

	CurrentGameplayActions currentActions;

	ProcessSlowMotion_gamepad(padState, currentActions, player);

	ProcessDownwardDash_gamepad(padState, currentActions, player);

	ProcessMelee_gamepad(padState, currentActions, player);

	ProcessCrouch_gamepad(padState, currentActions, player);

	ProcessRoll_gamepad(padState, currentActions, player);

	ProcessLeftRightMovement_gamepad(padState, currentActions, player);

	ProcessJump_gamepad(padState, currentActions, player);

	ProcessWallJump_gamepad(padState, currentActions, player);

	ProcessAimDirection_gamepad(padState, currentActions, player, levelProps);

	ProcessPrimaryWeapon_gamepad(padState, currentActions, player);

	ProcessSecondaryWeapon_gamepad(padState, currentActions, player);

	ProcessStrafing_gamepad(padState, currentActions, player, levelProps);

	ProcessSwimDown_gamepad(padState, currentActions, player, levelProps);

	ProcessTestActions_gamepad(padState, currentActions, player, levelProps);
}

void InputManager::ProcessGameplay_Keyboard()
{
	Player * player = GameObjectManager::Instance()->GetPlayer();

	if (!player)
	{
		return;
	}

	const auto & levelProps = GameObjectManager::Instance()->GetCurrentLevelProperties();

	CurrentGameplayActions currentActions;

	ProcessLeftRightMovement_keyboard(currentActions, player);

}

void InputManager::ProcessLeftRightMovement_keyboard(CurrentGameplayActions & currentActions, Player * player)
{
	if (GameObjectManager::Instance()->GetCurrentLevelProperties().IsAnimationPreview())
	{
		return;
	}

	if (!player->JustFellFromLargeDistance() &&
		!currentActions.mIsCrouching &&
		!player->GetIsRolling())
	{
		if (GetAsyncKeyState(VK_LEFT) < 0 &&
			!player->IsDoingMelee() &&
			!player->IsWallJumping())
		{
			player->AccelerateX(-100);
			currentActions.mIsSprinting = true;
		}
		else if (GetAsyncKeyState(VK_RIGHT) < 0 &&
			!player->IsDoingMelee() &&
			!player->IsWallJumping())
		{
			player->AccelerateX(100);
			currentActions.mIsSprinting = true;
		}
		else
		{
			// not pressing anything
			player->StopXAccelerating();
		}
	}

	player->SetSprintActive(currentActions.mIsSprinting);
}

bool InputManager::IsPressingEnterDoor() const
{
	GamePad * gamepad1 = GamePad::GetPad1();

	if (gamepad1 && gamepad1->IsConnected())
	{
		XINPUT_STATE padState = gamepad1->GetState();
		return padState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * 3.0f;
	}
	else
	{
		return GetAsyncKeyState(VK_UP) < 0;
	}
}