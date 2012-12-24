#include "StdAfx.h"
#include "FileIF.h"
#include "windows.h"
#include "StrSafe.h"

CFileIF::CFileIF(void)
{
}

CFileIF::~CFileIF(void)
{
}

BOOL CFileIF::FileExist(CString strFileName)
{
	CFileFind fFind;
	return fFind.FindFile(strFileName);
}

BOOL CFileIF::CreateFolder(CString strPath)
{
	//SECURITY_ATTRIBUTES attribu;
	return ::CreateDirectory(strPath,0);
}

void CFileIF::ErrorProcess(LPTSTR lpszFunction)
{
	LPVOID lpMsgBuf;
	//LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,dw,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL);

	//lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));   
	//StringCchPrintf((LPTSTR)lpDisplayBuf,LocalSize(lpDisplayBuf),TEXT("%s failed with error %d: %s"),lpszFunction, dw, lpMsgBuf); 

	// Display the error message and exit the process
	CString error_msg;
	error_msg.Format(_T("Error in %s"),lpszFunction);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, error_msg, MB_OK);

	LocalFree(lpMsgBuf);
	//LocalFree(lpDisplayBuf);
}