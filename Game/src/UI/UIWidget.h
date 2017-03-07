#ifndef UIWIDGET_H
#define UIWIDGET_H

struct EventStruct; // forward dec

class UIWidget
{
public:

	UIWidget(void);
	~UIWidget(void);

	virtual void XmlRead(TiXmlElement * element);
	virtual void OnFocus();
	virtual void OnLoseFocus();
	void HandleEvent(enum UIEventTypesEnum eventType);

	virtual void Release(); // release our memory for this object
	virtual void Initialise();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void Update();
	virtual void Draw(ID3D10Device * device);

	bool GetIsInFocus() { return mCurrentlyInFocus; }

	virtual void Reset() = 0;

	inline string Name()
	{
		return m_name;
	}
	inline void SetName(string name)
	{
		m_name = name;
	}
	inline Vector2 BottomLeft()
	{
		return m_bottomLeft;
	}
	inline void SetBottomLeft(Vector2 value)
	{
		m_bottomLeft = value;
	}
	inline Vector2 Dimensions()
	{
		return m_dimensions;
	}
	inline void SetDimensions(Vector2 value)
	{
		m_dimensions = value;
	}
	inline bool IsProcessInput()
	{
		return m_processInput;
	}
	inline  bool IsVisible()
	{
		return m_visible;
	}
	inline void SetIsProcessInput(bool value)
	{
		m_processInput = value;
	}
	inline void SetVisible(bool value)
	{
		m_visible = value;
	}

	inline void SetAlpha(float newAlpha ) { m_alpha = newAlpha; }
	inline float Alpha() { return m_alpha; }

	virtual void Scale(float x, float y);

	// override this o assign different UI actions for different type widgets
	// string eventType = "actionspressdown", "actionspressup" etc...
	virtual void AssignEventAction(string eventType, EventStruct eventStruct);

protected:

	Vector2 m_bottomLeft;
	Vector2 m_dimensions;
	bool m_processInput;
	bool m_visible;
	// the overall alpha value of the object
	float m_alpha;
	
	// event handler functions
	virtual void OnPressDown();
	virtual void OnPressUp();
	
	// the actions associated with this widget for pressing up and down
	list<EventStruct> m_pressUpActions;
	list<EventStruct> m_pressDownActions;

	bool mCurrentlyInFocus; // for gamepad

	string m_name;
};

#endif
