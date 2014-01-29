//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _STRING_UTILS_H
#define _STRING_UTILS_H

#include <stdarg.h>
#include "guideBot/platform.h"
////////////////////////////////////////////////////
///////*******String functions************//////////
////////////////////////////////////////////////////

namespace GuideBot
{

inline const char* format(const char *message, ...)
{
   static char buffer[4096];
   va_list args;
   va_start(args, message);
   vsnprintf(buffer, sizeof(buffer), message, (char*)args);
   va_end (args);
   return buffer;
}

inline const char *getWord(const char *string, size_t index, const char *set)
{
	static char buffer[1024];
	size_t sz;
	while(index--)
	{
		if(!*string)
			return "";
		sz = strcspn(string, set);
		if (string[sz] == 0)
			return "";
		string += (sz + 1);
	}
	sz = strcspn(string, set);
	if (sz == 0)
		return "";

	GB_ASSERT(sz + 1<sizeof( buffer), "Error! String buffer overflow" );

	char *ret = &buffer[0];
	strncpy(ret, string, sz);
	ret[sz] = '\0';
	return ret;
}

class EnumDescription
{
public:
	struct Record
	{
		int value;
		const char* name;
	};

	EnumDescription(size_t size,const Record* records): m_size(size), m_records(records)
	{
	}

	template<typename T>
	bool getValueByName(const char* name, T& value)
	{
		for (size_t i = 0; i<m_size; i++)
		{
			if (!strcmp(name, m_records[i].name))
			{
				value = (T) m_records[i].value;
				return true;
			}
		}
		return false;
	}

	const char* getNameByValue(int value)
	{
		for (size_t i = 0; i<m_size; i++)
		{
			if (value == m_records[i].value)
			{
				return m_records[i].name;
			}
		}
		return NULL;
	}
	
protected:
	size_t	m_size; 
	const Record* m_records;

};

};

#endif //_STRING_UTILS_H