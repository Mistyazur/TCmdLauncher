#pragma once
class SingleProc
{
public:
	SingleProc()
	{

	}

	~SingleProc()
	{
		close();
	}

	BOOL init(LPCTSTR lpName)
	{
		BOOL exist = TRUE;

		if (open(lpName, sizeof(BOOL), TRUE))
		{
			if (write(&exist, sizeof(BOOL)))
				return TRUE;
		}

		return FALSE;
	}

	BOOL exists(LPCTSTR lpName)
	{
		BOOL exist = FALSE;

		if (open(lpName, sizeof(BOOL), FALSE))
			read(&exist, sizeof(BOOL));

		close();

		return exist;
	}

private:
	BOOL open(
		LPCTSTR lpName,
		DWORD dwMaxSize,
		BOOL bCreate
	)
	{
		if (bCreate)
		{
			m_hMapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,	// use paging file
				NULL,                   // default security
				PAGE_READWRITE,         // read/write access
				0,                      // maximum object size (high-order DWORD)
				dwMaxSize,              // maximum object size (low-order DWORD)
				lpName);				// name of mapping object
		}
		else
		{
			m_hMapFile = OpenFileMapping(
				FILE_MAP_ALL_ACCESS,   // read/write access
				FALSE,                 // do not inherit the name
				lpName);
		}


		if (m_hMapFile == NULL)
			return FALSE;

		m_dwMaxSize = dwMaxSize;

		return TRUE;
	}

	BOOL close()
	{
		if (m_hMapFile == NULL)
			return TRUE;

		return CloseHandle(m_hMapFile);
	}

	BOOL read(
		LPVOID pBuf,
		DWORD dwSize
	)
	{
		LPVOID pMapBuf;

		if (m_hMapFile == NULL)
			return FALSE;

		pMapBuf = MapViewOfFile(
			m_hMapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			m_dwMaxSize
		);

		if (pMapBuf == NULL)
			return FALSE;

		memcpy(pBuf, pMapBuf, dwSize);

		UnmapViewOfFile(pMapBuf);

		return TRUE;
	}

	BOOL write(
		LPVOID pBuf,
		DWORD dwSize
	)
	{
		LPVOID pMapBuf;

		if (m_hMapFile == NULL)
			return FALSE;

		pMapBuf = MapViewOfFile(m_hMapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			m_dwMaxSize);

		if (pMapBuf == NULL)
			return FALSE;

		memcpy(pMapBuf, pBuf, dwSize);

		UnmapViewOfFile(pMapBuf);

		return TRUE;
	}
	
	HANDLE m_hMapFile;
	DWORD m_dwMaxSize;
};

