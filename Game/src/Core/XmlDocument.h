#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

class XmlDocument
{
public:
	XmlDocument(void);
	~XmlDocument(void);
	
	bool Load(const string & fileName, bool doBabbage = false);

	void Save(const string & filename,  TiXmlElement * root, bool doBabbage = false);
	
	TiXmlHandle * Handle()
	{
		return m_pHandle;
	}

	TiXmlElement * GetRoot()
	{
		return m_pHandle->FirstChildElement().Element();
	}

private:

	TiXmlDocument m_document;
	TiXmlHandle * m_pHandle;
};

#endif
