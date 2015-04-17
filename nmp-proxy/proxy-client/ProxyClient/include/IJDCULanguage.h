#ifndef __IJDCULANGUAGE_H__
#define __IJDCULANGUAGE_H__

class IJLanguage
{
public:
	virtual	void		JDelete() = 0;

	virtual long		JLoadFile(const char* szFile, BOOL bReload = FALSE) = 0;
	virtual CStringA	JGetTextA(const char* szTableName, const char* szNodeName) = 0;
	virtual CStringW	JGetTextW(const wchar_t* szTableName, const wchar_t* szNodeName) = 0;
};

IJLanguage* __stdcall JDCUCreateLanguage();
#endif