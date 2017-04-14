#include "precompiled.h"
#include "XmlDocument.h"

XmlDocument::XmlDocument(void)
{
}

XmlDocument::~XmlDocument(void)
{
}

bool XmlDocument::Load(const char * filename)
{
	bool loadOkay = m_document.LoadFile(filename);

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

void XmlDocument::Save(const char * filename, TiXmlElement * root)
{
	TiXmlDocument doc;
	doc.LinkEndChild( root );
	doc.SaveFile( filename );
}
