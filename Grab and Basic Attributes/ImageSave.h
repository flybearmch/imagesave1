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
	//��Ա����
	enum{
		copysize=200//200
	};
	CWinThread* ImageSaveThread;
	bool stopISThread;
	Image* imageCopy[copysize];
	Image* imageCopy2[copysize];//˫�������
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
	int copycount;//����ָʾ�����������飬ʹ����һ����˫�������
	CCriticalSection critical_section;
public:
	//��Ա����
	static UINT ImageSaveThreadFunc(LPVOID lpParam);
	UINT ImageSaveThreadFuncInternal();
	void StopImageSaveThread();
	void BeginThread();
	void DisplayNIIMAQdxError(IMAQdxError error);
	void DisplayNIVisionError(int error);
};
