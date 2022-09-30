#pragma once
#include "md5.h"
class CHash
{
public:
	CHash(void);
	~CHash(void);
	CString				HashString(CString strString);
};
