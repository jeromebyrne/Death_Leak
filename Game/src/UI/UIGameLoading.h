#ifndef UIGAMELOADING_H
#define UIGAMELOADING_H

#include "uiscreen.h"

class UISprite;

class UIGameLoading : public UIScreen
{

public:
	UIGameLoading(string name);
	virtual ~UIGameLoading(void);

	void StartLoading(const string & levelToLoad);

	virtual void Update() override;

	virtual void Initialise() override;

private:

	void Load();

	void FinishedLoading();

	void StartFadeIn();

	void StartFadeOut();

	void FinishedFadeOut();

	bool mHasStartedLoading = false;

	bool mHasStartedFadeIn = false;

	bool mHasStartedFadeOut = false;

	float mLoadTimeStartTime = 0.0f;

	float mStartFadeInTime = 0.0f;

	float mStartFadeOutTime = 0.0f;

	string mLevelToLoad;

	UISprite * mBGSprite = nullptr;
};

#endif
