#include "precompiled.h"
#include "XmlDocument.h"

XmlDocument::XmlDocument(void)
{
}

XmlDocument::~XmlDocument(void)
{
}

bool XmlDocument::Load(const string & filename)
{
	bool loadOkay = m_document.LoadFile(filename.c_str());

	if ( !loadOkay )
	{
		LOG_ERROR("Unable to open xml file %s. Check that the file exists", filename);
 		GAME_ASSERT(loadOkay);
	}
	else
	{
		// create the handle
		m_pHandle = new TiXmlHandle(&m_document);
	}

	return loadOkay;
}

void XmlDocument::Save(const string & filename, TiXmlElement * root)
{
	TiXmlDocument doc;
	doc.LinkEndChild( root );
	doc.SaveFile( filename.c_str() );
}
