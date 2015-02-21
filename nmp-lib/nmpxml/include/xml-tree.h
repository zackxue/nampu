

#ifndef __J_XML_TREE_H__
#define __J_XML_TREE_H__



#define MAX_MXML_NAME_LEN				64
#define MAX_MXML_ATTR_LEN				64
#define MAX_MXML_CONTACT_LEN			1024


#define DEF_MXML_HEAD_STR				"?xml version=\"1.0\" encoding=\"UTF-8\"?"
#define DEF_MXML_IGNORE_STR				"!--"
#define DEF_MXML_CDATA_STR				"![CDATA["


typedef struct __XmlTreeAttr
{
    char name[MAX_MXML_NAME_LEN];
    char value[MAX_MXML_ATTR_LEN];
	struct __XmlTreeAttr *prev;
	struct __XmlTreeAttr *next;
}XmlTreeAttr;

typedef struct __XmlTreeElement
{
	char name[MAX_MXML_NAME_LEN];
	char contact[MAX_MXML_CONTACT_LEN];
	char *extend_contact;
	XmlTreeAttr *attr;
}XmlTreeElement;

typedef struct __XmlTreeNode
{
	XmlTreeElement element;
	struct __XmlTreeNode *prev;
	struct __XmlTreeNode *next;
	struct __XmlTreeNode *parent;
	struct __XmlTreeNode *child;
}XmlTreeNode;

typedef enum __ContactType
{
	CONTACT_NULL=0,
	CONTACT_INT,
	CONTACT_CHAR,
	CONTACT_STRING,
}ContactType;


#endif	//__J_XML_TREE_H__

