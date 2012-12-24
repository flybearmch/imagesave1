//////////////////////////////////////////////////////////////////////////////
//
//  Title     : GrabBasicAttrDlg.cpp
//  Project   : NI-IMAQdx
//  Created   : 2/12/2008 @ 10:19:26
//  Platforms : All
//  Copyright : National Instruments 2006.  All Rights Reserved.
//  Access    : Public
//  Purpose   : Illustrates how to perform a simple, high-level grab acquisition,
//              with basic attribute support built using MSVC.
//
//////////////////////////////////////////////////////////////////////////////


//============================================================================
//  Includes
//============================================================================
#include "stdafx.h"
#include "GrabBasicAttr.h"
#include "GrabBasicAttrDlg.h"
#include "FileIF.h"
//#include "afxmt.h"
#include "iostream"
using namespace std;
//============================================================================
//  Debug defines
//============================================================================
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//============================================================================
//  VisionError - Type typedef is simply used to differentiate NI Vision errors
//      vs. driver errors during error handling.
//============================================================================
typedef int VisionError;


//============================================================================
//  Constants
//============================================================================
const char* InitialFrameRate = "0.000000";
const char* InitialBufferNumber = "0";
const char* DefaultAttributeRoot = "CameraAttributes";
const IMAQdxAttributeVisibility DefaultAttributeVisibility = IMAQdxAttributeVisibilityAdvanced;


int imageBufferNumber;
int grabcount;

//
SYSTEMTIME st;
int time1;
int time2;
int mtime1;
int mtime2;
int mmtime1;
int mmtime2;
//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::CGrabBasicAttrDlg
//
//  Description:
//      Constructs the main CGrabBasicAttrDlg.
//
//////////////////////////////////////////////////////////////////////////////
CGrabBasicAttrDlg::CGrabBasicAttrDlg(CWnd* pParent) :
    CDialog(CGrabBasicAttrDlg::IDD, pParent),
    session(0),
    image(NULL),
    grabThread(NULL),
    stopThread(false)
{
	//{{AFX_DATA_INIT(CGrabBasicAttrDlg)
    camName = _T("cam0");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_IMAQDX);
    imaqSetWindowThreadPolicy(IMAQ_SEPARATE_THREAD);

}

//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::DoDataExchange
//
//  Description:
//      Exchanges data between our member variables and the dialog.
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGrabBasicAttrDlg)
	DDX_Control(pDX, ID_SET, setButton);
	DDX_Control(pDX, IDC_FRAMESPERSEC, frameRateControl);
	DDX_Control(pDX, IDC_BUFFERNUM, bufferNumberControl);
	DDX_Control(pDX, IDC_ATTRIBUTEVALUE, attributeValueControl);
	DDX_Control(pDX, IDC_ATTRIBUTENAME, attributeNameControl);
	DDX_Control(pDX, ID_STOP, stopButton);
	DDX_Control(pDX, ID_GRAB, grabButton);
	DDX_Control(pDX, IDC_CAMNAME, camNameControl);
    DDX_Text(pDX, IDC_CAMNAME, camName);
	//}}AFX_DATA_MAP
}

//============================================================================
//  Message map
//============================================================================
BEGIN_MESSAGE_MAP(CGrabBasicAttrDlg, CDialog)
	//{{AFX_MSG_MAP(CGrabBasicAttrDlg)
	ON_BN_CLICKED(ID_QUIT, OnQuit)
	ON_BN_CLICKED(ID_GRAB, OnGrab)
	ON_BN_CLICKED(ID_STOP, OnStop)
	ON_CBN_SELCHANGE(IDC_ATTRIBUTENAME, OnChangeAttributeName)
	ON_BN_CLICKED(ID_SET, OnSetAttributeValue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::OnInitDialog
//
//  Description:
//      Message handler which is called when it's time to initialize the dialog.
//
//////////////////////////////////////////////////////////////////////////////
BOOL CGrabBasicAttrDlg::OnInitDialog()
{
    //------------------------------------------------------------------------
    //  Let the base method run & initialize our big & small icons.  Also,
    //  start out with the Stop button disabled and the Start button enabled.
    //------------------------------------------------------------------------
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    camNameControl.EnableWindow(TRUE);
    grabButton.EnableWindow(TRUE);
    stopButton.EnableWindow(FALSE);
    frameRateControl.EnableWindow(FALSE);
    frameRateControl.SetWindowText(InitialFrameRate);
    bufferNumberControl.EnableWindow(FALSE);
    bufferNumberControl.SetWindowText(InitialBufferNumber);
    attributeNameControl.EnableWindow(TRUE);
    attributeValueControl.EnableWindow(TRUE);
    setButton.EnableWindow(TRUE);

	grabcount=0;
	//for (int i=0;i<imageSaveThreadNum;i++)// 初始化
	//{
	//	imagesave[i]=new CImageSave();
	//}
	for (int i=0;i<imageSaveThreadNum;i++)
	{
		imagesave[i].ThreadCount=i;
	}

	CFileIF fFileIF;
	if (!fFileIF.FileExist(imagesave[0].FileDire))
	{
		if (!fFileIF.CreateFolder(imagesave[0].FileDire))
		{
			fFileIF.ErrorProcess(TEXT("OnInitDialog"));//错误处理 有哪些错误？
		}
	}

	try
	{
		image = imaqCreateImage (IMAQ_IMAGE_U8, 0);//IMAQ_IMAGE_RGB
		if (!image)
			throw VisionError();
		//--------------------------------------------------------------------
		//  Open an interface to our board, as given by the interface name.
		//--------------------------------------------------------------------
		camNameControl.GetWindowText(camName);
		IMAQdxError status = IMAQdxOpenCamera (camName, IMAQdxCameraControlModeController, &session);
		if (status)
			throw status;

		//--------------------------------------------------------------------
		//  Format attribute name control.
		//--------------------------------------------------------------------
		status = FormatAttributeNameControl();
		if (status)
			throw status;
	
		//初始化相机属性
		InitSetAtrriValue();
		//--------------------------------------------------------------------
		//  Now it's time to start the GrabBasicAttr.  If the acquisition starts
		//  properly, go ahead and start the grab thread.
		//--------------------------------------------------------------------
		


		//stopThread = false;
		//grabThread = AfxBeginThread (CGrabBasicAttrDlg::GrabThreadFunc, this);
		camNameControl.EnableWindow (FALSE);
		//grabButton.EnableWindow (FALSE);
		//stopButton.EnableWindow (TRUE);
		attributeNameControl.EnableWindow (TRUE);
		attributeValueControl.EnableWindow (TRUE);
		setButton.EnableWindow (TRUE);
		//startedSuccessfully = true;
	}
	catch (IMAQdxError error) {
		DisplayNIIMAQdxError (error);
	}
	catch (VisionError) {
		DisplayNIVisionError (imaqGetLastError());
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::OnGrab
//
//  Description:
//      Message handler which is called when the user single-clicks the "Grab"
//      button.  Here, we'll start the GrabBasicAttr.
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::OnGrab() 
{
    UpdateData (TRUE);
    bool startedSuccessfully = false;
    frameRateControl.SetWindowText(InitialFrameRate);
    bufferNumberControl.SetWindowText(InitialBufferNumber);

    try 
	{
 		//--------------------------------------------------------------------
		//  We need an image for display purposes.  
		//--------------------------------------------------------------------
		//image = imaqCreateImage (IMAQ_IMAGE_U8, 0);
		//if (!image)
		//	throw VisionError();

		//--------------------------------------------------------------------
        //  Open an interface to our board, as given by the interface name.
        //--------------------------------------------------------------------
        //camNameControl.GetWindowText(camName);
        //IMAQdxError status = IMAQdxOpenCamera (camName, IMAQdxCameraControlModeController, &session);
        //if (status)
        //    throw status;

        //--------------------------------------------------------------------
        //  Now it's time to start the GrabBasicAttr.  If the acquisition starts
        //  properly, go ahead and start the grab thread.
        //--------------------------------------------------------------------
        IMAQdxError status;
		status = IMAQdxConfigureGrab (session);
        if (status)
            throw status;

        //--------------------------------------------------------------------
        //  Format attribute name control.
        //--------------------------------------------------------------------
       // status = FormatAttributeNameControl();
       // if (status)
       //     throw status;

        
        stopThread = false;
        grabThread = AfxBeginThread (CGrabBasicAttrDlg::GrabThreadFunc, this);//,THREAD_PRIORITY_LOWEST
		//实时存储线程
		for (int i=0;i<imageSaveThreadNum;i++)
		{
			imagesave[i].stopISThread=false;
			imagesave[i].BeginThread();
		}
		
		//
		//
       // camNameControl.EnableWindow (FALSE);
        grabButton.EnableWindow (FALSE);
        stopButton.EnableWindow (TRUE);
       // attributeNameControl.EnableWindow (TRUE);
       // attributeValueControl.EnableWindow (TRUE);
       // setButton.EnableWindow (FALSE);
        startedSuccessfully = true;
    }
    catch (IMAQdxError error) {
        DisplayNIIMAQdxError (error);
    }
    catch (VisionError) {
        DisplayNIVisionError (imaqGetLastError());
    }

    //------------------------------------------------------------------------
    //  If the acquisition wasn't started properly, we need to clean up.
    //  Otherwise, just update the state of the buttons and let it run.
    //------------------------------------------------------------------------
    if (!startedSuccessfully) 
	{
        if (grabThread)
            StopGrabThread();
		for (int i=0;i<imageSaveThreadNum;i++)
		{
			if (imagesave[i].ImageSaveThread)
			{
				imagesave[i].StopImageSaveThread();
			}
		}
		
        IMAQdxCloseCamera (session);
        session = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::GrabThreadFunc
//
//  Description:
//      This is the main grab worker thread.  Its job is to get images from
//      the driver and display them.
//
//////////////////////////////////////////////////////////////////////////////
UINT CGrabBasicAttrDlg::GrabThreadFunc (LPVOID param) 
{
     return reinterpret_cast<CGrabBasicAttrDlg*>(param)->GrabThreadFuncInternal ();
}

UINT CGrabBasicAttrDlg::GrabThreadFuncInternal ()
{
	//------------------------------------------------------------------------
    //  Start grabbing images until either the user tells us to stop or we
    //  encounter an error.
    //------------------------------------------------------------------------
    static int count=0;
	IMAQdxError status = IMAQdxErrorSuccess;
    uInt32 lastBufferNumber = -1;
    DWORD lastTick = GetTickCount();

	GetSystemTime(&st);
	time1=st.wMinute;
	mtime1=st.wSecond;
	mmtime1=st.wMilliseconds;

    while (!stopThread) 
	{
        uInt32 bufferNumber;
        status = IMAQdxGrab (session, image, TRUE, &bufferNumber);
		if (status)
			break;
		grabcount++;
		//将接收到的图像保存到图像队列中
		//临界区

		//oldimageBufferNumber=imageBufferNumber;
		//imageBufferNumber=bufferNumber;
		//imagesave[count].ThreadCount=count;
		WaitForSingleObject(imagesave[count].semaphoreEmpty->m_hObject,NULL);//返回值很有用，可以达到我想要达到的想过，设定一定的等待时间，如果超时，那么和成功返回的返回值是不一样的，以此来判断下一步该怎么做；INFINITE
		if (imagesave[count].copycount==0)
		{
			imaqDuplicate(imagesave[count].imageCopy[imagesave[count].in_index],image);
		} 
		else
		{
			imaqDuplicate(imagesave[count].imageCopy2[imagesave[count].in_index],image);
		}
		
		imagesave[count].in_index=(imagesave[count].in_index+1)%imageCopyNum;
		if (imagesave[count].in_index==0)
		{
			imagesave[count].copycount=(imagesave[count].copycount+1)%2;
		}
		ReleaseSemaphore(imagesave[count].semaphoreFull->m_hObject,1,NULL);
		
		count=(count+1)%imageSaveThreadNum;
		//critical_section.Lock();
		//while(imageDup[imageDupNumTag]!=NULL && imageDupNumTag<ImageDupNum)
		//{
		//	imageDupNumTag++;
		//}
		//if (imageDupNumTag<ImageDupNum)
		//{
		//	if (!imaqDuplicate(imageDup[imageDupNumTag],image))
		//	{
				//错误处理
		//	}
		//	else
		//	{
		//		imageDupNumTag=(imageDupNumTag+1)%ImageDupNum;
		//	}
		//}
		
		//imageDup.push_back(imageCopy);

		//
		//critical_section.Unlock();
        //imaqDisplayImage(image, 0, true);
		GetSystemTime(&st);
		time2=st.wMinute;
		mtime2=st.wSecond;
		mmtime2=st.wMilliseconds;
		if ((time2-time1+60)%60>=2 && mtime1==mtime2 && (mmtime2-mmtime1+1000)%1000<=10)//1minute
		{
			OnStop();

			//m_running_chgstate.DoClick();
			//break;
		}
        //--------------------------------------------------------------------
        //  Every so often, update the dialog with the current frame rate and
		//  buffer number.  The frame rate calculation interface is given by 
		//	frameRateInterval, in seconds.  This is a running average of the 
		//	most recent N images within an interval.
        //--------------------------------------------------------------------
        const double frameRateInterval = 0.5;
        DWORD newTick = GetTickCount();
        if ((double)(newTick - lastTick) / 1000.0 >= frameRateInterval) //0.5s更新一次，但是最后停止的时候，显示的buffernumber应该是正确的,这个地方要更改
		{
            CString frameRateString, bufferNumberString;
            double frameRate = ((double)(bufferNumber - lastBufferNumber) / (double)(newTick - lastTick)) * 1000.0;
            frameRateString.Format("%.2f", frameRate);
            lastTick = newTick;
            lastBufferNumber = bufferNumber;
            bufferNumberString.Format("%u", bufferNumber);
            frameRateControl.SetWindowText(frameRateString);
            bufferNumberControl.SetWindowText(bufferNumberString);
        }
    }
    //------------------------------------------------------------------------
    //  Well, it's time to stop.  If we got an error, stop the acquisition.
    //  Afterwards tell the outside world that our thread is done.
    //------------------------------------------------------------------------
    if (status) 
	{
        DisplayNIIMAQdxError(status);
        SetActiveWindow();
        stopButton.PostMessage(BM_CLICK);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::OnStop
//
//  Description:
//      Message handler which is called when the user single-clicks the "Stop"
//      button.  Here, we'll stop the GrabBasicAttr.
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::OnStop() 
{
    //------------------------------------------------------------------------
    //  Stop the grab thread.  Stop the acquisition and close the session.
    //  Dispose of the image.
    //------------------------------------------------------------------------
    if (grabThread)
        StopGrabThread();
	
	int savecount;
	savecount=0;
	for (int i=0;i<imageSaveThreadNum;i++)
	{
		savecount+=imagesave[i].savecount;
	}
	//要等缓冲区里面的资源都被消耗完，再停止存储图片的线程
	Sleep(40000);
	
	for (int i=0;i<imageSaveThreadNum;i++)
	{
		if (imagesave[i].ImageSaveThread)
		{
			imagesave[i].StopImageSaveThread();
		}
	}
	
	

    if (session) 
	{
		IMAQdxCloseCamera (session);
        session = 0;
    }
    
	if (image)
	{
	    imaqDispose (image);
		image = NULL;
	}
	
	
	

	//检测是否实时
	
	if (grabcount!=0)
	{
		CString temp;
		temp.Format(_T("%d %d"),grabcount,savecount);
		MessageBox(temp);
	}
	
	grabcount=0;

	//for (int i=0;i<imageSaveThreadNum;i++)//注销资源，调用析构函数
	//{
	//	imagesave[i].~CImageSave();
	//}

	camNameControl.EnableWindow(TRUE);
    grabButton.EnableWindow(TRUE);
    stopButton.EnableWindow(FALSE);
    attributeNameControl.EnableWindow(FALSE);
    attributeValueControl.EnableWindow(FALSE);
    setButton.EnableWindow(FALSE);
}

//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::OnQuit
//
//  Description:
//      Message handler which is called when the user single-clicks the "Quit"
//      button.
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::OnQuit() 
{
    //------------------------------------------------------------------------
    //  Stop the acquisition and clean up the image display.
    //------------------------------------------------------------------------
    if (grabcount!=0)
    {
		OnStop();
    }
	//OnStop();
    imaqShowWindow(0, false);
	OnCancel();
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::StopGrabThread
//
//  Description:
//      Stops the grab thread and destroys the object associated with it.
//      You should only call this function if the thread was properly created.
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::StopGrabThread() {
    //------------------------------------------------------------------------
    //  Tell the thread to stop and wait for it to do so.  Note that the timeout
    //  should be longer than your frame time to guarantee that the thread
    //  stops by the time we exit this function.
    //------------------------------------------------------------------------
    stopThread = true;
    WaitForSingleObject(grabThread->m_hThread, 5000);
    grabThread = NULL;
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::DisplayNIIMAQdxError
//
//  Description:
//      Displays a message box containing the NI-IMAQdx error information.
//
//  Parameters:
//      error - The error code
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::DisplayNIIMAQdxError(IMAQdxError error) {
    //--------------------------------------------------------------------
    //  Display a dialog containing the NI-IMAQdx error we encountered.
    //--------------------------------------------------------------------
    Int8 errorText[512];
    sprintf(errorText, "Error Code = 0x%08X\n\n", error);
	IMAQdxGetErrorString (error, errorText + strlen(errorText), sizeof(errorText) - strlen(errorText));
    MessageBox(errorText, "NI-IMAQdx Error");
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::DisplayNIVisionError
//
//  Description:
//      Displays a message box containing the NI Vision error information.
//
//  Parameters:
//      error - The error code
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::DisplayNIVisionError(int error) {
    //--------------------------------------------------------------------
    //  Display a dialog containing the NI Vision error we encountered.
    //--------------------------------------------------------------------
    char* errorText = imaqGetErrorText(error);
    char dialogText[512];
    sprintf(dialogText, "Error Code = 0x%08X\n\n%s", error, errorText);
    MessageBox(dialogText, "NI Vision Error");
    imaqDispose(errorText);
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::FormatAttributeNameControl
//
//  Description:
//      Format attribute name control with enumerated attributes
//
//  Return Value:
//      Error code
//
//////////////////////////////////////////////////////////////////////////////
IMAQdxError CGrabBasicAttrDlg::FormatAttributeNameControl() {
    IMAQdxError status = IMAQdxErrorSuccess;
    //--------------------------------------------------------------------
    //  Clear current combo box
    //--------------------------------------------------------------------
    attributeNameControl.ResetContent();
    //--------------------------------------------------------------------
    //  Enumerate available camera attributes
    //--------------------------------------------------------------------
    uInt32 attributeCount = 0;
    IMAQdxAttributeInformation* attributeArray = NULL;
    status = IMAQdxEnumerateAttributes2(session, NULL, &attributeCount, DefaultAttributeRoot, DefaultAttributeVisibility);
    if (status)
        return status;
    attributeArray = new IMAQdxAttributeInformation[attributeCount];
    if (!attributeArray)
        return IMAQdxErrorSystemMemoryFull;
    status = IMAQdxEnumerateAttributes2(session, attributeArray, &attributeCount, DefaultAttributeRoot, DefaultAttributeVisibility);
    if (status)
        return status;
    //--------------------------------------------------------------------
    //  Add camera attribute names to combo box
    //--------------------------------------------------------------------
    for (uInt32 i = 0; i < attributeCount; ++i) {
        attributeNameControl.AddString(attributeArray[i].Name);
    }
    //--------------------------------------------------------------------
    //  Select the first item
    //--------------------------------------------------------------------
    if (attributeCount) {
        attributeNameControl.SetCurSel(0);
        OnChangeAttributeName();
    }
    //--------------------------------------------------------------------
    //  Delete attribute array
    //--------------------------------------------------------------------
    delete [] attributeArray;
    return status;
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::OnChangeAttributeName
//
//  Description:
//      Event handler for selecting a new attribute name.
//      Query attribute value and update corresponding control.
//
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::OnChangeAttributeName() 
{
    CString attributeName;
    attributeNameControl.GetWindowText(attributeName);
    //--------------------------------------------------------------------
    //  Query read/write access
    //--------------------------------------------------------------------
    bool32 readable = false, writable = false;
    IMAQdxIsAttributeReadable(session, attributeName, &readable);
    IMAQdxIsAttributeWritable(session, attributeName, &writable);
    //--------------------------------------------------------------------
    //  Query current value
    //--------------------------------------------------------------------
    char buffer[IMAQDX_MAX_API_STRING_LENGTH] = { 0 };
    if (readable) {
        IMAQdxGetAttribute(session, attributeName, IMAQdxValueTypeString, buffer);
    }
    //--------------------------------------------------------------------
    //  Update attribute value control
    //--------------------------------------------------------------------
    attributeValueControl.EnableWindow(readable && writable);
    attributeValueControl.SetWindowText(buffer);
    setButton.EnableWindow(writable);
}


//////////////////////////////////////////////////////////////////////////////
//
//  CGrabBasicAttrDlg::OnSetAttributeValue
//
//  Description:
///     Event handler for changing attribute value.
//      
//////////////////////////////////////////////////////////////////////////////
void CGrabBasicAttrDlg::OnSetAttributeValue() 
{
    CString attributeName, attributeValue;
    attributeNameControl.GetWindowText(attributeName);
    attributeValueControl.GetWindowText(attributeValue);
    //--------------------------------------------------------------------
    //  Set current value
    //--------------------------------------------------------------------
    IMAQdxError status = IMAQdxSetAttribute(session, attributeName, IMAQdxValueTypeString, (const char*)attributeValue);
    if (status) {
        DisplayNIIMAQdxError(status);
    }
    //--------------------------------------------------------------------
    //  Query coerced value
    //--------------------------------------------------------------------
    char buffer[IMAQDX_MAX_API_STRING_LENGTH] = { 0 };
    IMAQdxGetAttribute(session, attributeName, IMAQdxValueTypeString, buffer);
    attributeValueControl.SetWindowText(buffer);
}

void CGrabBasicAttrDlg::InitSetAtrriValue()
{
	CString attributeName_Height, attributeValue_Height,attributeName_ExposureTime,attributeValue_ExposureTime,attributeName_Rate,attributeValue_Rate,attributeName_PixelFormat,attributeName_Gain,attributeValue_Gain;
	//attributeNameControl.GetWindowText(attributeName);
	//attributeValueControl.GetWindowText(attributeValue);
	attributeName_Height.Format(_T("CameraAttributes::ImageFormatControl::Height"));
	attributeValue_Height.Format(_T("100"));
	attributeName_ExposureTime.Format(_T("CameraAttributes::AcquisitionControl::ExposureTime"));
	attributeValue_ExposureTime.Format(_T("5"));
	attributeName_Rate.Format(_T("CameraAttributes::AcquisitionControl::AcquisitionFrameRate"));
	attributeValue_Rate.Format(_T("200"));
	attributeName_PixelFormat.Format(_T("CameraAttributes::ImageFormatControl::PixelFormat"));
	attributeName_Gain.Format(_T("CameraAttributes::AnalogControl::Gain"));
	attributeValue_Gain.Format(_T("6"));
	//--------------------------------------------------------------------
	//  Set current value
	//--------------------------------------------------------------------
	IMAQdxError status = IMAQdxSetAttribute(session, attributeName_Height, IMAQdxValueTypeString, (const char*)attributeValue_Height);
	if (status) {
		DisplayNIIMAQdxError(status);
	}
	status = IMAQdxSetAttribute(session, attributeName_ExposureTime, IMAQdxValueTypeString, (const char*)attributeValue_ExposureTime);
	if (status) {
		DisplayNIIMAQdxError(status);
	}
	status = IMAQdxSetAttribute(session, attributeName_Rate, IMAQdxValueTypeString, (const char*)attributeValue_Rate);
	if (status) {
		DisplayNIIMAQdxError(status);
	}
	status = IMAQdxSetAttribute(session, attributeName_Gain, IMAQdxValueTypeString, (const char*)attributeValue_Gain);
	if (status) {
		DisplayNIIMAQdxError(status);
	}

	IMAQdxEnumItem temp2[3];
	uInt32 size=3;
	IMAQdxEnumerateAttributeValues(session,(const char*)attributeName_PixelFormat,temp2,&size);
	//temp.Name=(char*)(LPCTSTR)attributeValue_PixelFormat;
	//strncpy(temp.Name,(LPCTSTR)attributeValue_PixelFormat,sizeof(temp.Name));
	status = IMAQdxSetAttribute(session, attributeName_PixelFormat, IMAQdxValueTypeEnumItem, temp2[0]);//Mono8 Mono12Packed BayerRG8
	if (status) {
		DisplayNIIMAQdxError(status);
	}

	status = IMAQdxSetAttribute(session, attributeName_Height, IMAQdxValueTypeString, (const char*)attributeValue_Height);
	if (status) {
		DisplayNIIMAQdxError(status);
	}
}