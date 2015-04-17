#ifndef __IJDCUPARSEXML_H__
#define __IJDCUPARSEXML_H__

#define XMLMSG		"message"
#define TYPE		"type"
#define RET			"resultCode"

class IJXmlDocument;
class IJXmlElement;
class IJXmlAttribute;
class ICuConfig;

class IJXmlParser
{
public:
	virtual void					JDelete() = 0;
	virtual IJXmlDocument*			JCreateXmlDocument() = 0;
	virtual IJXmlElement*			JCreateXmlElement() = 0;
	//virtual IJXmlAttribute*			JCreateXmlAttribute() = 0;
	virtual ICuConfig*				JCreateConfig() = 0;
	virtual ICuConfig*				JCreateGlobalConfig() = 0;
	virtual IJXmlElement*			JCreateMsgDoc(IJXmlDocument*& pDoc, const char* szType) = 0;
	virtual IJXmlElement*			JAppendChildEle(IJXmlElement* pParentEle, const char* szName, const char* szValue) = 0;
	virtual long					JGetRetCode(IJXmlElement* pMsgEle, const char* szName = RET) = 0;
};

IJXmlParser* __stdcall JDCUCreateParser();

class IJXmlElement
{
public:
	virtual void			JDelete() = 0;
	virtual IJXmlDocument*	GetDocument() = 0;

	// @param szName NULL：表示获取第一个儿子结点；不为NULL：表示查找指点儿子结点。
	virtual IJXmlElement*	FirstChild(const char* szName = 0) = 0;
	virtual IJXmlElement*	LastChild(const char* szName = 0) = 0;
	virtual IJXmlElement*	NextSibling() = 0;
	virtual IJXmlElement*	PreviousSibling() = 0;
	virtual IJXmlElement*	Clone() = 0;

	virtual IJXmlAttribute*	FirstAttribute() = 0;
	virtual IJXmlAttribute*	LastAttribute() = 0;

	virtual long			InsertChild(IJXmlElement* pInsert, IJXmlElement* pBefore = 0) = 0;
	virtual long			RemoveChild(IJXmlElement* pChild) = 0;

	virtual long			SetName(const char* szName) = 0;
	virtual const char*		GetName() = 0;
	virtual long			SetText(const char* szText, bool bCDATA = false) = 0;
	virtual const char*		GetText(bool* pbCDATA = 0) = 0;

	virtual long			SetAttribute(const char* szAttr, const char* szValue) = 0;
	virtual const char*		GetAttribute(const char* szAttr) = 0;
};

class IJXmlDocument : public IJXmlElement
{
public:
	virtual void			JDelete() = 0;
	virtual long			LoadFile(const char* szFile) = 0;
	virtual long			SaveFile(const char* szFile) = 0;
	
	virtual long			LoadXML(const char* szXml) = 0;
	virtual long			GetXML(char* szXml, int nSize) = 0;

	virtual long			SetDeclaration(const char* szVersion, const char* szEncoding, const char* szStandalone) = 0;
	virtual const char*		GetVersion() = 0;
	virtual const char*		GetEncoding() = 0;
	virtual const char*		GetStandalone() = 0;	
};

class IJXmlAttribute
{
public:
	virtual void			JDelete() = 0;
	virtual IJXmlAttribute*	Next() = 0;
    virtual IJXmlAttribute* Previous() = 0;
	virtual long            SetName(const char* szName) = 0;
    virtual const char*     GetName() = 0;
    virtual long            SetValue(const char* szValue) = 0;
    virtual const char*     GetValue() = 0;
};

class ICuConfig
{
public:
	virtual void JDelete() = 0;
	virtual long OpenFile(const char* szFileName) = 0;
	virtual const char* GetKeyValue(const char* szAppName, const char* szKeyName) = 0;
	virtual long SetKeyValue(const char* szAppName, const char* szKeyName, const char* szValue) = 0;
};

#endif