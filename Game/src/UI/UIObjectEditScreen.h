#ifndef UIOBJECTEDITSCREEN_H
#define UIOBJECTEDITSCREEN_H

#include "uiscreen.h"

class UITextBox;

class UIObjectEditScreen : public UIScreen
{
public:
	UIObjectEditScreen(string name);
	virtual ~UIObjectEditScreen(void);

	virtual void Update();

	virtual void Initialise();

	void SetObjectToEdit(GameObject * object);

private:

	void addTextBoxesForCurrentObject();

	void removeTexBoxes();

	void addRow(TiXmlElement * xmlElement, float startX, float startY, unsigned int & textBoxCountOut, unsigned int rowCount);

	GameObject * mCurrentObject = nullptr;

	std::map<std::string, UITextBox*> mTextBoxMap;
};

#endif
