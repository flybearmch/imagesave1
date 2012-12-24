#pragma once

class CFileIF
{
public:
	CFileIF(void);
public:
	~CFileIF(void);
public:
	BOOL FileExist(CString strFileName);
	BOOL CreateFolder(CString strPath);
	void ErrorProcess(LPTSTR lpszFunction);
};
