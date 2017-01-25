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

	void ApplyChanges();

private:

	void clearData();

	void AddTextBoxesForObject(GameObject * object);

	TiXmlElement * ConvertPropertiesToXmlElement();

	void AddRow(TiXmlElement * xmlElement, float startX, float startY, unsigned int & textBoxCountOut, unsigned int rowCount);

	void UpdateObjectProperties();

	GameObject * mCurrentObject = nullptr;

	vector<pair<std::string, std::map<std::string, std::string>>> mObjectProperties;
};

#endif
