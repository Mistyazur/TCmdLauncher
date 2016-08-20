#pragma once

#include <Shobjidl.h>
#include <shellapi.h>
#include <propsys.h>
#include <Propkey.h.>
#include <Propvarutil.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Propsys.lib")

class Launcher
{
public:

	static BOOL initialize()
	{
		return SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
	}

	static VOID uninitialize()
	{
		CoUninitialize();
	}

	static BOOL setAppId(LPCTSTR szAppID)
	{
		HRESULT hr = SetCurrentProcessExplicitAppUserModelID(szAppID);
		if (SUCCEEDED(hr))
			return TRUE;

		return FALSE;
	}

	static BOOL getLnkAppPath(LPCTSTR szLnk, LPTSTR szPath, UINT cch)
	{
		HRESULT hr;
		IShellLink* link;
		IPersistFile* file;

		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link));
		if (SUCCEEDED(hr))
		{
			hr = link->QueryInterface(IID_PPV_ARGS(&file));
			if (SUCCEEDED(hr))
			{
				hr = file->Load(szLnk, STGM_READ | STGM_SHARE_DENY_NONE);
				if (SUCCEEDED(hr))
				{
					link->GetPath(szPath, cch, NULL, SLGP_RAWPATH);
				}
				file->Release();
			}
			link->Release();
		}

		return SUCCEEDED(hr);
	}

	static BOOL setLnkAppPath(LPCTSTR szLnk, LPTSTR szPath)
	{
		HRESULT hr;
		IShellLink* link;
		IPersistFile* file;

		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link));
		if (SUCCEEDED(hr))
		{
			hr = link->QueryInterface(IID_PPV_ARGS(&file));
			if (SUCCEEDED(hr))
			{
				hr = file->Load(szLnk, STGM_READWRITE);
				if (SUCCEEDED(hr))
				{
					hr = link->SetPath(szPath);
					if (SUCCEEDED(hr))
					{
						hr = file->Save(NULL, TRUE);
					}
				}
				file->Release();
			}
			link->Release();
		}

		return SUCCEEDED(hr);
	}

	static BOOL getLnkAppID(LPCTSTR szLnk, LPTSTR szAppID, UINT cch)
	{
		HRESULT hr;
		IShellLink* link;
		IPersistFile* file;
		IPropertyStore* store;
		PROPVARIANT pv;

		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link));
		if (SUCCEEDED(hr))
		{
			hr = link->QueryInterface(IID_PPV_ARGS(&file));
			if (SUCCEEDED(hr))
			{
				hr = file->Load(szLnk, STGM_READ | STGM_SHARE_DENY_NONE);
				if (SUCCEEDED(hr))
				{
					hr = link->QueryInterface(IID_PPV_ARGS(&store));
					if (SUCCEEDED(hr))
					{
						hr = store->GetValue(PKEY_AppUserModel_ID, &pv);
						if (SUCCEEDED(hr))
						{
							if (pv.vt != VT_EMPTY && pv.vt == VT_LPWSTR)
							{
								_tcscpy_s(szAppID, cch, pv.pwszVal);
							}
							PropVariantClear(&pv);
						}
						store->Release();
					}
				}
				file->Release();
			}
			link->Release();
		}

		return SUCCEEDED(hr);
	}

	static BOOL setLnkAppID(LPCTSTR szLnk, LPTSTR szAppID)
	{
		HRESULT hr;
		IShellLink* link;
		IPersistFile* file;
		IPropertyStore* store;
		PROPVARIANT pv;

		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link));
		if (SUCCEEDED(hr))
		{
			hr = link->QueryInterface(IID_PPV_ARGS(&file));
			if (SUCCEEDED(hr))
			{
				hr = file->Load(szLnk, STGM_READWRITE);
				if (SUCCEEDED(hr))
				{
					hr = link->QueryInterface(IID_PPV_ARGS(&store));
					if (SUCCEEDED(hr))
					{
						pv.vt = VT_LPWSTR;
						pv.pwszVal = szAppID;
						hr = store->SetValue(PKEY_AppUserModel_ID, pv);
						if (SUCCEEDED(hr))
						{
							// Not sure if we need to do this
							pv.pwszVal = NULL;
							PropVariantClear(&pv);

							hr = store->Commit();
							if (SUCCEEDED(hr))
							{
								hr = file->Save(NULL, TRUE);
							}
						}
						store->Release();
					}
				}
				file->Release();
			}
			link->Release();
		}

		return SUCCEEDED(hr);
	}

	static BOOL getWndAppID(HWND hWnd, LPTSTR szAppID, UINT cch)
	{
		HRESULT hr;
		IPropertyStore *pPS = NULL;
		PROPVARIANT pv;

		hr = SHGetPropertyStoreForWindow(hWnd, IID_PPV_ARGS(&pPS));
		if (SUCCEEDED(hr))
		{
			hr = pPS->GetValue(PKEY_AppUserModel_ID, &pv);
			if (SUCCEEDED(hr) && pv.vt != VT_EMPTY)
			{
				hr = PropVariantToString(pv, szAppID, cch);
				if (SUCCEEDED(hr))
					return TRUE;
			}
		}

		return FALSE;
	}

	static BOOL setWndAppID(HWND hWnd, LPCTSTR szAppID)
	{
		HRESULT hr;
		IPropertyStore *pPS = NULL;
		PROPVARIANT pv;

		if (szAppID != NULL)
		{
			hr = SHGetPropertyStoreForWindow(hWnd, IID_PPV_ARGS(&pPS));
			if (SUCCEEDED(hr))
			{
				hr = InitPropVariantFromString(szAppID, &pv);
				if (SUCCEEDED(hr))
				{
					hr = pPS->SetValue(PKEY_AppUserModel_ID, pv);
					if (SUCCEEDED(hr))
					{
						pPS->Commit();
						return TRUE;
					}
					PropVariantClear(&pv);
				}
				pPS->Release();
			}
		}

		return FALSE;
	}

private:
	Launcher()
	{
	}

	~Launcher()
	{
	}
};