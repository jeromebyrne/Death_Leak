#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

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

		char* boolAsString = ReadAttributeAsString(element, subElementName, key);
		Utilities::ToLower(boolAsString); // convert to lower

		if(strcmp(boolAsString, "true") == 0)
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
		float value = Utilities::ConvertStringToDouble(floatAsString);

		return value;
	}

	static char* ReadAttributeAsString(TiXmlElement * element, const char* subElementName, const char * key)
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
		
		char* returnString = (char*)attributeElement->Attribute(key);
		return returnString;
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
};

#endif