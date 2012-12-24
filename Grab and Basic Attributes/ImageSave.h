#pragma once

#include "NIIMAQdx.h"
#include "nivision.h"
#include "afxmt.h"

class CImageSave
{
public:
	CImageSave(void);
public:
	~CImageSave(void);
public:
	//成员变量
	enum{
		copysize=200//200
	};
	CWinThread* ImageSaveThread;
	bool stopISThread;
	Image* imageCopy[copysize];
	Image* imageCopy2[copysize];//双缓冲机制
	CSemaphore *semaphoreFull;
	CSemaphore *semaphoreEmpty;
	int in_index;
	int out_index;
	int savecount;
	int imageCopyNum;
	int ThreadCount;
	//int ImageCountInThread;
	CString FileDire;
	unsigned char* imagearray[200]; 
	int copycount;//用于指示两个保存数组，使用哪一个，双缓冲机制
	CCriticalSection critical_section;
public:
	//成员函数
	static UINT ImageSaveThreadFunc(LPVOID lpParam);
	UINT ImageSaveThreadFuncInternal();
	void StopImageSaveThread();
	void BeginThread();
	void DisplayNIIMAQdxError(IMAQdxError error);
	void DisplayNIVisionError(int error);
};
