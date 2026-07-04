#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <string>
#include "tinyxml.h"

class XmlUtilities
{
public:

	XmlUtilities(void)
	{
	}

	~XmlUtilities(void)
	{
	}
	
	static bool ReadAttributeAsBool(TiXmlElement * element, const char* subElementName, const char * key)
	{
		bool returnValue = false;

		const char * boolAsString = ReadAttributeAsString(element, subElementName, key);
		std::string lowerValue = ToLowerCopy(boolAsString);

		if(lowerValue == "true" || lowerValue == "1")
		{
			returnValue = true;
		}

		return returnValue;
	}

	static float ReadAttributeAsFloat(TiXmlElement * element, const char* subElementName, const char * key)
	{
		TiXmlElement * attributeElement;

		if(strcmp(subElementName, ""))
		{
			attributeElement = element->FirstChildElement(subElementName);
		}
		else
		{
			attributeElement = element;
		}

		const char* floatAsString = attributeElement->Attribute(key);
		float value = static_cast<float>(ConvertStringToDouble(floatAsString));

		return value;
	}

	static int ReadAttributeAsInt(TiXmlElement * element, const char* subElementName, const char * key)
	{
		TiXmlElement * attributeElement;

		if (strcmp(subElementName, ""))
		{
			attributeElement = element->FirstChildElement(subElementName);
		}
		else
		{
			attributeElement = element;
		}

		const char* intAsString = attributeElement->Attribute(key);
		int value = intAsString != nullptr ? std::atoi(intAsString) : 0;

		return value;
	}

	static const char * ReadAttributeAsString(TiXmlElement * element, const char* subElementName, const char * key)
	{
		TiXmlElement * attributeElement;

		// if it is an empty string then we just use the parent element
		if(strcmp(subElementName, ""))
		{
			attributeElement = element->FirstChildElement(subElementName);
		}
		else
		{
			attributeElement = element;
		}
		
		return attributeElement->Attribute(key);
	}

	static TiXmlElement * GetChildElement(TiXmlElement * element, const char * childName)
	{
		return element->FirstChildElement(childName);
	}

	static bool AttributeExists(TiXmlElement * element, const char * childName, const char * attributeName)
	{
		TiXmlElement * attributeElement;
		if(strcmp(childName, ""))
		{
			attributeElement = element->FirstChildElement(childName);
		}
		else
		{
			attributeElement = element;
		}

		if (attributeElement->Attribute(attributeName) != nullptr)
		{
			return true;
		}

		return false;
	}

private:
	static std::string ToLowerCopy(const char* value)
	{
		std::string lowerValue = value != nullptr ? value : "";
		for (char& c : lowerValue)
		{
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		return lowerValue;
	}

	static double ConvertStringToDouble(const char* str)
	{
		if (str == nullptr)
		{
			return 0.0;
		}

		static const unsigned int kBufferSize = 256;
		char buf[kBufferSize];
		std::strncpy(buf, str, kBufferSize);
		buf[kBufferSize - 1] = '\0';

		char* dot = std::strchr(buf, '.');
		if (dot != nullptr && dot - buf + 8 < kBufferSize)
		{
			dot[8] = '\0';
		}

		return std::atof(buf);
	}
};

#endif
