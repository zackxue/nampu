
#include "JString.h"
#include "stdafx.h"

void JString::MakeLower()
{
	for(size_t i = 0;i < length();i++)
	{
		if(at(i) > 0)
		{
			if(!islower(at(i)))
			{
				at(i) = tolower(at(i));
			}
		}
	}
}

void JString::MakeUpper()
{
	for(size_t i = 0;i < length();i++)
	{
		if(at(i) > 0)
		{
			if(!isupper(at(i)))
			{
				at(i) = toupper(at(i));
			}
		}
	}
}

void JString::TrimLeft(char trim)
{
	for(size_t i = 0;i < length();i++)
	{
		if(at(i) != trim)
		{
			*this = substr(i,-1);
			return;
		}
	}

	*this = "";
}

int JString::Compare(const JString& str) const
{
	return compare(str);
}

int JString::CompareNoCase(const JString& str) const
{
	return _stricmp(c_str(),str.c_str());
}

void JString::Replace(const JString& strOld , const JString& strNew)
{
	for(;;)
	{
		string::size_type pos(0);
		if((pos = find(strOld)) != string::npos)
		{
			replace(pos,strOld.length(),strNew);
		}
		else
		{
			break;
		}
	}
}

int JString::Find(const JString& str,int iStart) const
{
	return find(str,iStart);
}

JString JString::Left(int nCount) const
{
	return substr(0,nCount);
}

JString JString::Right(int nCount) const
{
	int nLen = length();
	if(nCount >= nLen)
	{
		return *this;
	}
	else
	{
		return substr(nLen - nCount, nCount);
	}	
}

JString JString::Mid(int nPlace, int nCount) const
{
	return substr(nPlace,nCount);
}

void JString::Format(const char* szFormat, ...)
{
	char buf[4096];
	va_list ap;
	va_start(ap , szFormat);
	int len = vsnprintf(buf,sizeof(buf) - 1 ,szFormat,ap);
	va_end(ap);
	if(len < 0)
	{
		len = 0;
	}
	if(len > sizeof(buf) - 1)
	{
		len = sizeof(buf) - 1;
	}
	buf[len] = 0;

	*this = buf;
}

void JString::Decompose(const JString& split, vector<JString>& arStr) const
{
	arStr.clear();

	int pS = 0;
	int pE = find(split);
	
	while(pE >= 0)
	{
		arStr.push_back(substr(pS,pE - pS));
		pS = pE + split.length();
		pE = find(split,pS);
	}

	if(pS < (int)length())
	{
		arStr.push_back(substr(pS));
	}

}

LONG JString::ToLong() const
{
	LONG value = 0;
	sscanf(c_str(),"%d",(int*)&value);
	return value;
}

LONGLONG JString::ToLongLong() const
{
	LONGLONG value = 0;
	sscanf(c_str(),"%I64d",&value);
	return value;
}
