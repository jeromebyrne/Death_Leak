#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

class XmlDocument
{
private:
	TiXmlDocument m_document;
	TiXmlHandle * m_pHandle;
public:
	XmlDocument(void);
	~XmlDocument(void);
	
	bool Load(const string & fileName);

	void Save(const string & filename,  TiXmlElement * root);
	
	TiXmlHandle * Handle()
	{
		return m_pHandle;
	}

	TiXmlElement * GetRoot()
	{
		return m_pHandle->FirstChildElement().Element();
	}
};

#endif
