#ifndef UISCREEN_H
#define UISCREEN_H
#include "uiwidget.h"

class UIScreen
{
public:
	UIScreen(string name);
	~UIScreen(void);
	
	string Name() { return m_name; }
	virtual void XmlRead(TiXmlElement * element);
	virtual void Initialise();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void Update();
	virtual void Draw(ID3D10Device * device);
	virtual void Release();

	// applies this alpha value to every widget on the screen
	void ApplyAlpha(float newAlpha);
	
	// scale all of the widgets on the screen - for resolution changes
	void ScaleScreen(float scaleFactorX, float scaleFactorY);

	bool GetShowCursor() const { return mShowCursor; }

protected:

	map<string, UIWidget*> m_widgetMap; // a map of all the widgets that this screen holds

	bool IsPointWithinBounds(float pointx, float pointy, Vector2 topLeftBounds, Vector2 boundsDimensions);

	void ResetFocusedWidget() { m_currentWidgetInFocus = nullptr; }

private: 

	void ProcessCursorInput(); // process input for this screen
	void ProcessKeyStrokes(); // listen for any keystrokes for registered buttons
	void ProcessGamePad(); // listen for gamepad button presses

	UIWidget * CreateWidget(const char * type);

	string m_name; // the name of this screen
	UIWidget * m_currentWidgetInFocus; // the current widget in focus
	Vector2 m_lastMousePos; // the position of the mouse the last time we checked
	bool mShowCursor;
};

#endif
