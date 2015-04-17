/************************************************************************ 
* 文件名：    User.h  
* 文件描述：  定义了用户管理的一些方法
* 创建人：    XiaoHe, 2013年03月2日
* 版本号：    1.0 
************************************************************************/ 

#pragma once

class CUser
{
private:
	CString m_name;
	CString m_pwd;
	CString m_newpwd;

public:
	IJXmlParser *m_pXmlParser;
	IJDCUCommand *m_pCommand;

public:
	CUser(void);
	~CUser(void);
	int SetName(CString);
	CString GetName();

	int SetPwd(CString);
	CString GetPwd();

	int SetNewPwd(CString);
	CString GetNewPwd();

	int Add(CString name,CString pwd);
	int Del(CString name);
	int Modify(CString newpwd);
};
