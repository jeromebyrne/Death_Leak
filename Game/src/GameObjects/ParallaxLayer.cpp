#include "precompiled.h"
#include "ParallaxLayer.h"

ParallaxLayer::ParallaxLayer(Camera2D * camera):Sprite(), 
	m_camera(camera), 
	m_repeatWidth(128), 
	m_cameraParallaxMultiplierX(0),
	m_cameraParallaxMultiplierY(0), 
	m_followCamYPos(false), 
	m_followCamXPos(true), 
	m_autoScrollX(false), 
	m_autoScrollY(false), 
	m_autoScrollXSpeed(0), 
	m_autoScrollYSpeed(0), 
	m_lastCamPosY(0)
{
	mIsParallaxLayer = true;
}

ParallaxLayer::~ParallaxLayer(void)
{
}

void ParallaxLayer::Update(float delta)
{
	Sprite::Update(delta);
	
	float camX = m_camera->X();
	float camY = m_camera->Y();

	if(m_followCamXPos){  m_position.X = camX; }
	if(m_followCamYPos){  m_position.Y = camY; }
}

void ParallaxLayer::Draw(ID3D10Device * device, Camera2D * camera)
{
	float camX = m_camera->X();
	float camY = m_camera->Y();
	Timing * timing = Timing::Instance();

	if(m_cameraParallaxMultiplierX > 0 || m_autoScrollX)
	{
		float viewAreaX = camera->ViewWidth() * 0.5f;
		float repeatsX = (viewAreaX * 2.0f) / m_repeatWidth;
		repeatsX = 1.0f;
		float xShift = 0;
		
		if(m_autoScrollX)
		{
			float targetDelta =  timing->GetTargetDelta();
			float percentDelta = timing->GetLastUpdateDelta() / targetDelta;

			if (m_currentEffectType != EFFECT_BUMP)
			{
				xShift += m_vertices[0].TexCoord.x + (m_autoScrollXSpeed * 0.001f) * percentDelta;
			}
			else
			{
				xShift +=  m_verticesBump[0].TexCoord.x + (m_autoScrollXSpeed * 0.001f) * percentDelta;
			}
		}
		else
		{
			if (m_horizontalFlip)
			{
				xShift += camX * m_cameraParallaxMultiplierX;
			}
			else
			{
				xShift += camX * -m_cameraParallaxMultiplierX;
			}
		}

		// figure out the x tex coords for each vertex
		if (m_currentEffectType != EFFECT_BUMP)
		{
			m_vertices[0].TexCoord.x = xShift + repeatsX;
			m_vertices[1].TexCoord.x = xShift;
			m_vertices[2].TexCoord.x = xShift;
			m_vertices[3].TexCoord.x = xShift + repeatsX;
		}
		else
		{
			m_verticesBump[0].TexCoord.x = xShift + repeatsX ;
			m_verticesBump[1].TexCoord.x = xShift;
			m_verticesBump[2].TexCoord.x = xShift;
			m_verticesBump[3].TexCoord.x = xShift + repeatsX;
		}
		
		// have to do this to due to unidentified bug with y tex coords
		if(m_cameraParallaxMultiplierY <= 0 && !m_autoScrollY)
		{
			if (m_currentEffectType != EFFECT_BUMP)
			{
				m_vertices[0].TexCoord.y = 0.0f;
				m_vertices[1].TexCoord.y = 0.0f;
				m_vertices[2].TexCoord.y = 1.0f;
				m_vertices[3].TexCoord.y = 1.0f;
			}
			else
			{
				m_verticesBump[0].TexCoord.y = 0.0f;
				m_verticesBump[1].TexCoord.y = 0.0f;
				m_verticesBump[2].TexCoord.y = 1.0f;
				m_verticesBump[3].TexCoord.y = 1.0f;
			}
		}

		m_applyChange = true; // need to reset vertex buffer due to tex coord change
	}

	if(m_cameraParallaxMultiplierY > 0 || m_autoScrollY)
	{
		float repeatsY = 1;
		float yShift = 0;
		
		if(m_autoScrollY)
		{
			float targetDelta =  timing->GetTargetDelta();
			float percentDelta = timing->GetLastUpdateDelta() / targetDelta;

			if (m_currentEffectType != EFFECT_BUMP)
			{
				yShift = m_vertices[0].TexCoord.y + (m_autoScrollYSpeed * 0.001f) * percentDelta;
			}
			else
			{
				yShift = m_verticesBump[0].TexCoord.y + (m_autoScrollYSpeed * 0.001f) * percentDelta;
			}
		}
		else
		{
			yShift = camY * -m_cameraParallaxMultiplierY;
		}

		if (m_currentEffectType != EFFECT_BUMP)
		{
			//figure out the y tex coords for each vertex
			m_vertices[0].TexCoord.y = yShift;
			m_vertices[1].TexCoord.y = yShift;
			m_vertices[2].TexCoord.y = yShift + repeatsY;
			m_vertices[3].TexCoord.y = yShift + repeatsY;
		}
		else
		{
			//figure out the y tex coords for each vertex
			m_verticesBump[0].TexCoord.y = yShift;
			m_verticesBump[1].TexCoord.y = yShift;
			m_verticesBump[2].TexCoord.y = yShift + repeatsY;
			m_verticesBump[3].TexCoord.y = yShift + repeatsY;
		}

		m_applyChange = true; // need to reset vertex buffer due to tex coord change
	}
		
	m_lastCamPosY = camY;
	Sprite::Draw(device, camera);

}
void ParallaxLayer::XmlRead(TiXmlElement * element)
{
	// read base class
	Sprite::XmlRead(element);

	m_cameraParallaxMultiplierX = XmlUtilities::ReadAttributeAsFloat(element, "cameraparallaxmultiplierx", "value");
	m_cameraParallaxMultiplierY = XmlUtilities::ReadAttributeAsFloat(element, "cameraparallaxmultipliery", "value");
	m_repeatWidth = XmlUtilities::ReadAttributeAsFloat(element, "repeatwidth", "value");
	m_followCamYPos = XmlUtilities::ReadAttributeAsBool(element, "followcamy", "value");
	m_followCamXPos = XmlUtilities::ReadAttributeAsBool(element, "followcamx", "value");
	m_autoScrollX = XmlUtilities::ReadAttributeAsBool(element, "autoscrollx", "value");
	m_autoScrollY = XmlUtilities::ReadAttributeAsBool(element, "autoscrolly", "value");
	m_autoScrollXSpeed = XmlUtilities::ReadAttributeAsFloat(element, "autoscrollxspeed", "value");
	m_autoScrollYSpeed = XmlUtilities::ReadAttributeAsFloat(element, "autoscrollyspeed", "value");
}

void ParallaxLayer::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * cameraparallaxmultiplierxElem = new TiXmlElement("cameraparallaxmultiplierx");
	cameraparallaxmultiplierxElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_cameraParallaxMultiplierX).c_str());
	element->LinkEndChild(cameraparallaxmultiplierxElem);

	TiXmlElement * cameraparallaxmultiplieryElem = new TiXmlElement("cameraparallaxmultipliery");
	cameraparallaxmultiplieryElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_cameraParallaxMultiplierY).c_str());
	element->LinkEndChild(cameraparallaxmultiplieryElem);

	TiXmlElement * repeatwidth = new TiXmlElement("repeatwidth");
	repeatwidth->SetAttribute("value", Utilities::ConvertDoubleToString(m_repeatWidth).c_str());
	element->LinkEndChild(repeatwidth);

	TiXmlElement * followcamy = new TiXmlElement("followcamy");
	const char * followcamyAsStr = m_followCamYPos ? "true" : "false";
	followcamy->SetAttribute("value", followcamyAsStr);
	element->LinkEndChild(followcamy);

	TiXmlElement * followcamx = new TiXmlElement("followcamx");
	const char * followcamxAsStr = m_followCamXPos ? "true" : "false";
	followcamx->SetAttribute("value", followcamxAsStr);
	element->LinkEndChild(followcamx);

	TiXmlElement * autoscrollx = new TiXmlElement("autoscrollx");
	const char * autoscrollxAsStr = m_autoScrollX ? "true" : "false";
	autoscrollx->SetAttribute("value", autoscrollxAsStr);
	element->LinkEndChild(autoscrollx);

	TiXmlElement * autoscrolly = new TiXmlElement("autoscrolly");
	const char * autoscrollyAsStr = m_autoScrollY ? "true" : "false";
	autoscrolly->SetAttribute("value", autoscrollyAsStr);
	element->LinkEndChild(autoscrolly);

	TiXmlElement * autoscrollxspeed = new TiXmlElement("autoscrollxspeed");
	autoscrollxspeed->SetAttribute("value", Utilities::ConvertDoubleToString(m_autoScrollXSpeed).c_str());
	element->LinkEndChild(autoscrollxspeed);

	TiXmlElement * autoscrollyspeed = new TiXmlElement("autoscrollyspeed");
	autoscrollyspeed->SetAttribute("value", Utilities::ConvertDoubleToString(m_autoScrollYSpeed).c_str());
	element->LinkEndChild(autoscrollyspeed);
}

void ParallaxLayer::Scale(float x, float y, bool scalePosition)
{
	Sprite::Scale(x, y, scalePosition);

	m_repeatWidth = m_repeatWidth * x;
	if (m_cameraParallaxMultiplierX > 0)	m_cameraParallaxMultiplierX = m_cameraParallaxMultiplierX / x;
	if (m_cameraParallaxMultiplierY > 0)	m_cameraParallaxMultiplierY = m_cameraParallaxMultiplierY / y;
	if (m_autoScrollXSpeed > 0)				m_autoScrollXSpeed = m_autoScrollXSpeed * x;
	if (m_autoScrollYSpeed > 0)				m_autoScrollYSpeed = m_autoScrollYSpeed * y;
}
