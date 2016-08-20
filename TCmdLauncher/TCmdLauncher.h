#pragma once

#include "resource.h"

class TCmdLauncher
{
public:
	TCmdLauncher();
	~TCmdLauncher();
	BOOL exec();
private:
	VOID autoFreeUserCheck();
};