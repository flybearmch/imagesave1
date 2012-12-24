#include "stdafx.h"
#include "ImageSave.h"
#include "iostream"
using namespace std;
typedef int VisionError;

CImageSave::CImageSave(void)
{
	stopISThread=false;
	ImageSaveThread=NULL;
	
	semaphoreFull=new CSemaphore(0,copysize);
	semaphoreEmpty=new CSemaphore(copysize,copysize);
	savecount=0;
	in_index=0;
	out_index=0;
	ThreadCount=0;
	copycount=0;
	//ImageCountInThread=0;
	try
	{
		for (int i=0;i<copysize;i++)
		{
			imageCopy[i] = imaqCreateImage (IMAQ_IMAGE_U8, 0);//IMAQ_IMAGE_RGB
			imageCopy2[i] = imaqCreateImage (IMAQ_IMAGE_U8, 0);
			if ((!imageCopy[i]) || (!imageCopy2[i]))
				throw VisionError();
		}
	}
	catch (IMAQdxError error) {
		DisplayNIIMAQdxError (error);
	}
	catch (VisionError) {
		DisplayNIVisionError (imaqGetLastError());
	}
	//ImageSaveThread=AfxBeginThread(CImageSave::ImageSaveThreadFunc,this);
	
	FileDire.Format(_T("c:\\imagestore3"));
}


void CImageSave::StopImageSaveThread() {
	//要等缓冲区里的文件保存完毕，再停止保存线程
	
	stopISThread = true;
	//也许缓冲区里面还有没完成写操作的文件 这个需要注意
	WaitForSingleObject(ImageSaveThread->m_hThread, 5000);
	ImageSaveThread = NULL;
}

void CImageSave::BeginThread()
{
	ImageSaveThread=AfxBeginThread(CImageSave::ImageSaveThreadFunc,this);//,THREAD_PRIORITY_HIGHEST
}

UINT CImageSave::ImageSaveThreadFunc(LPVOID lpParam)
{
	return reinterpret_cast<CImageSave*>(lpParam)->ImageSaveThreadFuncInternal();
}

UINT CImageSave::ImageSaveThreadFuncInternal ()
{
	//临界区
	//static int count=0;
	//SYSTEMTIME st;
	CString fileName;
	Image* imagetemp=NULL;
	while(!stopISThread)
	{
		//critical_section.Lock();
		WaitForSingleObject(semaphoreFull->m_hObject,INFINITE);
		imagetemp=imageCopy[out_index];
		out_index=(out_index+1)%copysize;
		ReleaseSemaphore(semaphoreEmpty->m_hObject,1,NULL);

		if (imagetemp)
		{
			//if (count<2)
			//{
			//GetSystemTime(&st);
			fileName.Format(_T("%s\\%d_%d.jpg"),FileDire,ThreadCount,savecount);
			//int cols,rows;
			if (imaqWriteJPEGFile(imagetemp,(LPCSTR)fileName,750,NULL))//imaqWriteBMPFile(imagetemp,(LPCSTR)fileName,FALSE,NULL))//imaqWriteJPEGFile(imagetemp,(LPCSTR)fileName,750,NULL)
			{
				savecount++;				
				//imaqImageToArray(imagetemp,IMAQ_NO_RECT,&cols,&rows);
			}
			else
			{
				//写图像文件错误处理
				DisplayNIVisionError (imaqGetLastError());
			}

			//销毁资源 是否必要？
			//imaqDispose(imagetemp);
			//imagetemp=NULL;
		}	

	}
	return 0;
}

void CImageSave::DisplayNIIMAQdxError(IMAQdxError error) {
	//--------------------------------------------------------------------
	//  Display a dialog containing the NI-IMAQdx error we encountered.
	//--------------------------------------------------------------------
	Int8 errorText[512];
	sprintf(errorText, "Error Code = 0x%08X\n\n", error);
	IMAQdxGetErrorString (error, errorText + strlen(errorText), sizeof(errorText) - strlen(errorText));
//	MessageBox(errorText, "NI-IMAQdx Error");
}

void CImageSave::DisplayNIVisionError(int error) {
	//--------------------------------------------------------------------
	//  Display a dialog containing the NI Vision error we encountered.
	//--------------------------------------------------------------------
	char* errorText = imaqGetErrorText(error);
	char dialogText[512];
	sprintf(dialogText, "Error Code = 0x%08X\n\n%s", error, errorText);
//	MessageBox(dialogText, "NI Vision Error");
	imaqDispose(errorText);
}

CImageSave::~CImageSave(void)
{
	delete semaphoreFull;
	delete semaphoreEmpty;
	for (int i=0;i<copysize;i++)
	{
		if (imageCopy[i])
		{
			imaqDispose (imageCopy[i]);
			imageCopy[i]=NULL;
		}
		if (imageCopy2[i])
		{
			imaqDispose (imageCopy2[i]);
			imageCopy2[i]=NULL;
		}
	}
}