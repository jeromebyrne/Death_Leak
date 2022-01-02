#include "precompiled.h"
#include "UIGameLoading.h"
#include "UIManager.h"
#include "Game.h"
#include "UISprite.h"

static const float kMinTimeToLoad = 0.05f;
static const float kFadeTime = 0.20f;

UIGameLoading::UIGameLoading(string name) :
	UIScreen(name)
{
}

UIGameLoading::~UIGameLoading(void)
{
}

void UIGameLoading::Initialise()
{
	UIScreen::Initialise();

	for (const auto & kvp : m_widgetMap)
	{
		UISprite * s = dynamic_cast<UISprite*>(kvp.second);

		if (s)
		{
			if (s->Name().find("1_load_splash") != std::string::npos)
			{
				mBGSprite = s;
				// mBGSprite->SetAlpha(0.0f);

				break;
			}
		}
	}
}

void UIGameLoading::StartLoading(const string & levelToLoad)
{
	mLevelToLoad = levelToLoad;
	mBGSprite->SetAlpha(0.0f);

	// reset all variables
	{
		mHasStartedLoading = false;
		mHasStartedFadeIn = false;
		mHasStartedFadeOut = false;
		mLoadTimeStartTime = 0.0f;
		mStartFadeInTime = 0.0f;
		mStartFadeOutTime = 0.0f;
	}

	Game::GetInstance()->PauseGame(false);

	StartFadeIn();
}

void UIGameLoading::Update()
{
	float currentTotalTime = Timing::Instance()->GetTotalTimeSeconds();

	if (mHasStartedFadeIn)
	{
		float alpha = (currentTotalTime - mStartFadeInTime) / kFadeTime;
		mBGSprite->SetAlpha(alpha);

		if (currentTotalTime > mStartFadeInTime + kFadeTime)
		{
			mBGSprite->SetAlpha(1.0f); 
			Load();
			mHasStartedFadeIn = false;
		}
	}
	else if (mHasStartedLoading)
	{
		if (currentTotalTime > mLoadTimeStartTime + kMinTimeToLoad)
		{
			FinishedLoading();
			mHasStartedLoading = false;
		}
	}
	else if (mHasStartedFadeOut)
	{
		float alpha = 1.0f - ((currentTotalTime - mStartFadeOutTime) / kFadeTime);
		mBGSprite->SetAlpha(alpha);
		if (currentTotalTime > mStartFadeOutTime + kFadeTime)
		{
			FinishedFadeOut();
			mHasStartedFadeOut = false;
		}
	}
}

void UIGameLoading::FinishedLoading()
{
	mHasStartedLoading = false;

	mLevelToLoad = "";

	Game::GetInstance()->UnPauseGame(false);
}

void UIGameLoading::Load()
{
	mHasStartedLoading = true;
	mLoadTimeStartTime = Timing::Instance()->GetTotalTimeSeconds();

	auto gom = GameObjectManager::Instance();

	gom->DeleteGameObjects();

	gom->LoadObjectsFromFile(mLevelToLoad);

	StartFadeOut();
}

void UIGameLoading::StartFadeIn()
{
	mHasStartedFadeIn = true;
	mStartFadeInTime = Timing::Instance()->GetTotalTimeSeconds();
}

void UIGameLoading::StartFadeOut()
{
	mHasStartedFadeOut = true;
	mStartFadeOutTime = Timing::Instance()->GetTotalTimeSeconds();
}

void UIGameLoading::FinishedFadeOut()
{
	mBGSprite->SetAlpha(0.0f);
	UIManager::Instance()->PopUIDeferred("gameloading");
}



