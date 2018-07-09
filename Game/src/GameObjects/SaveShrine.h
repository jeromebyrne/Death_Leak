#ifndef SAVESHRINE_H
#define SAVESHRINE_H

#include "gameobject.h"

class SaveShrine : public GameObject
{
public:
	SaveShrine(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 1.0f, float height = 1.0f);
	virtual ~SaveShrine();

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	void DisplaySaveText();

	void DisplaySaveParticles();

	bool mHasSaved = false;

	float mTimeUntilCanSaveAgain = 0.0f;
};

#endif

