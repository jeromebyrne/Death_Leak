#ifndef SAVESHRINE_H
#define SAVESHRINE_H

#include "gameobject.h"

class SaveShrine : public GameObject
{
public:
	SaveShrine(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1);
	virtual ~SaveShrine();

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	void DisplaySaveText();

	bool mHasSaved = false;
	bool mHasPlayerExitedBounds = true;

	float mTimeUntilCanSaveAgain = 0.0f;
};

#endif

