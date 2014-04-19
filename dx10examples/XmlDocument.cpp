#include "precompiled.h"
#include "XmlDocument.h"

XmlDocument::XmlDocument(void)
{
}

XmlDocument::~XmlDocument(void)
{
}

void XmlDocument::Load(const char * filename)
{
	bool loadOkay = m_document.LoadFile(filename);

	if ( !loadOkay )
	{
		printf( "Could not load xml file. Error='%s'. Exiting.\n", m_document.ErrorDesc() );
	}
	else
	{
		// create the handle
		m_pHandle = new TiXmlHandle(&m_document);
	}
}

void XmlDocument::Save(const char * filename, TiXmlElement * root)
{
	TiXmlDocument doc;
	doc.LinkEndChild( root );
	doc.SaveFile( filename );
}
