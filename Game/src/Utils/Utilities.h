#ifndef UTILITIES_H
#define UTILITIES_H

#include "gameobject.h"
#include "solidmovingsprite.h"
#include "camera2d.h"
#include <sstream>

const unsigned int kMaxFormatStringBufferSize = 64000;

class Utilities
{
public:

	Utilities(void)
	{
	}

	~Utilities(void)
	{
	}

	static Vector2 WorldToScreen(Vector2 worldPos)
	{
		Camera2D * cam = Camera2D::GetInstance();

		Vector2 pos = Vector2(worldPos.X - cam->Left(), cam->Top() - worldPos.Y);

		return pos;
	}

	static Vector2 ScreenToWorld(Vector2 screenPos)
	{
		Camera2D * cam = Camera2D::GetInstance();

		Vector2 pos = Vector2(cam->Left() + screenPos.X, cam->Top() - screenPos.Y);

		return pos;
	}

	// is a game object in a rectangle - uses objects m_dimensions to determine
	static bool IsObjectInRectangle(GameObject * object,float rectCentreX, float rectCentreY, float rectWidth, float rectHeight)
	{
		bool inRect = false;
	
		float objectX = object->X();
		float objectY = object->Y();
		float rectLeft = (rectCentreX - rectWidth/2);
		float rectRight = (rectCentreX + rectWidth/2);
		float rectBottom = (rectCentreY - rectHeight/2);
		float rectTop = (rectCentreY + rectHeight/2);

		// first check if the objects origin is in the zone
		if(objectX > rectLeft && objectX < rectRight &&
		   objectY > rectBottom && objectY < rectTop)
		{
			inRect = true;
		}
		// origin might not be in view but object could still be partially in view
		if(!inRect)
		{
			float objectRight = object->Right();
			float objectLeft = object->Left();
			float objectTop = object->Top();
			float objectBottom = object->Bottom();

			// check if we are partially in view on the x plane
			if(objectRight > rectLeft && objectLeft < rectRight)
			{
				// now check if we are partially in view on the y plane
				if(objectTop > rectBottom && rectTop < rectTop)
				{
					// we are viewable
					inRect = true;
				}
			}
		}

		return inRect;
	}

	// Is a solid object in rectanle - uses the solid objects Collision dimensions rather than regualr dimensions
	static bool IsSolidSpriteInRectangle(SolidMovingSprite * object,float rectCentreX, float rectCentreY, float rectWidth, float rectHeight)
	{
		bool inRect = false;
	
		float objectX = object->X();
		float objectY = object->Y();
		float rectLeft = (rectCentreX - rectWidth/2);
		float rectRight = (rectCentreX + rectWidth/2);
		float rectBottom = (rectCentreY - rectHeight/2);
		float rectTop = (rectCentreY + rectHeight/2);

		// first check if the objects origin is in the zone
		if(objectX > rectLeft && objectX < rectRight &&
		   objectY > rectBottom && objectY < rectTop)
		{
			inRect = true;
		}
		// origin might not be in view but object could still be partially in view
		if(!inRect)
		{
			float objectRight = object->CollisionRight();
			float objectLeft = object->CollisionLeft();
			float objectTop = object->CollisionTop();
			float objectBottom = object->CollisionBottom();

			// check if we are partially in view on the x plane
			if(objectRight > rectLeft && objectLeft < rectRight)
			{
				// now check if we are partially in view on the y plane
				if(objectTop > rectBottom && rectTop < rectTop)
				{
					// we are viewable
					inRect = true;
				}
			}
		}

		return inRect;
	}

	static wchar_t* ConvertCharStringToWcharString(const char* charString)
	{
		size_t charSize = strlen(charString) + 1;
		const size_t newSize = strlen(charString) * sizeof(wchar_t);
		size_t convertedChars = 0;
		wchar_t* wcstring = new wchar_t[newSize];
		mbstowcs_s(&convertedChars, wcstring , charSize, charString, _TRUNCATE);

		return wcstring;
	}

	static void ToUpper(char* myString)
	{
		unsigned char c;
		while (*myString) 
		{ 
			c = *myString; 
			*myString = toupper(c); 
			myString++;
		}
	}

	static void ToLower(char* myString)
	{
		unsigned char c;
		while (*myString) 
		{ 
			c = *myString; 
			*myString = tolower(c); 
			myString++;
		}
	}

	static std::string ConvertDoubleToString(double value)
	{
		std::ostringstream strs;
		strs << value;
		std::string str = strs.str();

		return str;
	}

	static double ConvertStringToDouble(const char * value)
	{
		return atof(value);
	}

	static string getFormattedString(const char * stringToFormat, ...)
	{
		if (!stringToFormat)
		{
			return "";
		}

		char buffer[kMaxFormatStringBufferSize];
		va_list args;
		va_start (args, stringToFormat);
		vsnprintf(buffer, kMaxFormatStringBufferSize, stringToFormat, args);
		va_end(args);

		return buffer;
	}
};

#endif
