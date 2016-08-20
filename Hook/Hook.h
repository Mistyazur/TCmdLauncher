
#pragma once
#ifndef _HOOK_H_
#define _HOOK_H_

#include "TCmd.h"

extern "C" __declspec(dllexport) BOOL InstallHook(DWORD dwPID, LPCSTR szConFile);
extern "C" __declspec(dllexport) BOOL UninstallHook();

#endif