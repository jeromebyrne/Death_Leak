#ifndef PARALLAXLAYER_H
#define PARALLAXLAYER_H

#include "sprite.h"
#include "Camera2D.h"

class ParallaxLayer : public Sprite
{
protected:
	Camera2D * m_camera; // we need to store a pointer to the main camera for updating positions
	float m_repeatWidth; // how big each repeat of the texture should be on the x axis
	float m_cameraParallaxMultiplierX; // how much the camera movement affects parallax scrolling on the X axis
	float m_cameraParallaxMultiplierY; // how much the camera movement affects parallax scrolling on the Y axis
	bool m_followCamXPos; // should we set the x position of the layer = to camera x position
	bool m_followCamYPos; // should we set the y position of the layer = to camera y position
	bool m_autoScrollX; // keep scrolling in the x direction, regardless of camera movement
	bool m_autoScrollY; // keep scrolling in the y direction, regardless of camera movement
	float m_autoScrollXSpeed;
	float m_autoScrollYSpeed;
	float m_lastCamPosY; // where was the y position of the camera in the last frame
public:

	friend class WeatherManager;
	friend class GameObjectManager;

	ParallaxLayer(Camera2D * camera);
	virtual ~ParallaxLayer(void);
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void Update(float delta) override;
	virtual void Scale(float x, float y, bool scalePosition = true) override;
	virtual void DebugDraw(ID3D10Device *  device) { /*Don't draw debug info for particles as it's too confusing*/ }

	bool FollowCamX () const { return m_followCamXPos; }
};

#endif
