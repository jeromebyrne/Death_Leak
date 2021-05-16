#include "precompiled.h"
#include "XmlDocument.h"
#include "encryption.h"

string kKey = "babson";

XmlDocument::XmlDocument(void)
{
}

XmlDocument::~XmlDocument(void)
{
}

bool XmlDocument::Load(const string & filename, bool doBabbage)
{
	bool loadOkay = false;
	if (doBabbage)
	{
		TiXmlDocument d;
		bool loaded = d.LoadFile(filename.c_str());
		if (!loaded)
		{
			return false;
		}

		TiXmlElement* rootContent = d.FirstChildElement();

		string content = rootContent->Attribute("z");

		string decrypted = decrypt(content, kKey);

		m_document.Parse(decrypted.c_str(), 0, TIXML_ENCODING_UTF8);

		m_pHandle = new TiXmlHandle(&m_document);

		return true;
	}
	else
	{
		loadOkay = m_document.LoadFile(filename.c_str());

		if (loadOkay)
		{
			// create the handle
			m_pHandle = new TiXmlHandle(&m_document);
		}
	}

	return loadOkay;
}

void XmlDocument::Save(const string & filename, TiXmlElement * root, bool doBabbage)
{
	TiXmlDocument doc;

	if (doBabbage)
	{
		// Declare a printer    
		TiXmlPrinter printer;

		// attach it to the document you want to convert in to a std::string 
		root->Accept(&printer);

		string xmlAsStr = printer.CStr();

		string encrypted = encrypt(xmlAsStr, kKey);

		TiXmlElement * newElement = new TiXmlElement("a");
		newElement->SetAttribute("z", encrypted.c_str());

#if _DEBUG
		newElement->SetAttribute("content", xmlAsStr.c_str());
#endif

		doc.LinkEndChild(newElement);


	}
	else
	{
		doc.LinkEndChild(root);
	}

	doc.SaveFile( filename.c_str() );
}
