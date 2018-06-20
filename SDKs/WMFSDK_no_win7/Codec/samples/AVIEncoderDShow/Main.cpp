
//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <mediaobj.h>
#include <dshow.h>
#include <stdio.h>
#include "resource.h" 

#include <streams.h>
#include <strmif.h>
#include <atlbase.h>
#include <dmodshow.h>
#include <qedit.h>
#include <objbase.h>

#include "wmcodecconst.h"
#include "wmcodeciface.h"
#include "videoenc.h"
#include "macros.h"



HINSTANCE hInst = 0;
HWND g_hwnd = 0;

VideoEncParams g_VideoEncodingParams;



//Set this flag to enable additional debug status messages
BOOL g_fVerbose = TRUE;

const int g_nOutputStringBufferSize = 1024 * sizeof (_TCHAR);
int g_OutputStringLen = 0;
_TCHAR szSource[_MAX_PATH]= {'\0'};
_TCHAR szTarget[_MAX_PATH] = {'\0'};
_TCHAR szProfile[_MAX_PATH] = {'\0'};
_TCHAR szTitle[_MAX_PATH] = {'\0'};
_TCHAR szAuthor[_MAX_PATH] = {'\0'};
_TCHAR szOutputWindow[g_nOutputStringBufferSize] = {'\0'};



    


// General filter graph creation functions
HRESULT CreateFilterGraph(IGraphBuilder **pGraph);
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter);
HRESULT SetNoClock(IFilterGraph *pGraph);
LONG WaitForCompletion(HWND ghwnd, IGraphBuilder *pGraph );

HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin** ppPin);
HRESULT GetPinByMajorType(IBaseFilter *pFilter, PIN_DIRECTION PinDir, GUID majortype, IPin** ppPin);
HRESULT ConnectFilters(IBaseFilter*, IBaseFilter*, IGraphBuilder*, AM_MEDIA_TYPE* pmt);
HRESULT AddNewFilter(REFCLSID clsid, IBaseFilter **ppFilter, IGraphBuilder* pGB, LPCWSTR pFilterName);
HRESULT GetInputMediaType(IGraphBuilder* pGB, IBaseFilter* pSplitter, AM_MEDIA_TYPE& mt);
HRESULT GetNextUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin** ppPin);
BOOL OpenFileDialog(HWND hWnd);

//Functions specific to using WMV DMO Encoder in DirectShow
HRESULT MakeWMVAVIFile(_TCHAR* szSource, _TCHAR* szTarget, _TCHAR* szProfile);
HRESULT ConfigureDMO(IMediaObject* pDMO, AM_MEDIA_TYPE* pmtIn, AM_MEDIA_TYPE* pmtOut);
HRESULT GetComplexitySettings(DWORD* pLive, DWORD* pOffline, DWORD* pMax);
void SetEncodingDefaults(HWND hDialog);

float GetDlgFloat(HWND hDlg, int dlgItem);
void  SetDlgFloat(HWND hDlg, int dlgItem, float value);


// Status messages in user interface
void OutputMsg(_TCHAR* msg);


//------------------------------------------------------------------------------
// Name: MakeWMVAVIFile()
// Desc: Play the file through a DirectShow filter graph that terminates in the File Writer.
//       We build a new filter graph manager each time inside this function. Each filter
//		 is created explicitly by the application and added to the graph. It is not required to
//       build the graph in this way, but we do so here to demonstrate the exact order
//       of operations for adding and configuring the DMO.
//------------------------------------------------------------------------------
HRESULT MakeWMVAVIFile(_TCHAR* szSource, _TCHAR* szTarget, _TCHAR* szProfile)
{
    HRESULT hr;    
    WCHAR wszTargetFile[_MAX_PATH] = {'\0'};
    WCHAR wszSourceFile[_MAX_PATH] = {'\0'};
    WCHAR wszAuthor[_MAX_PATH] = {'\0'};
    WCHAR wszTitle[_MAX_PATH] = {'\0'};
    
    CComPtr <IGraphBuilder>    pGraph;
    CComPtr <IBaseFilter>      pSourceFilter;
    CComPtr<IBaseFilter>       pAviSplitter;
    CComPtr <IBaseFilter>      pDMOWrapper;
    CComPtr<IBaseFilter>       pAviMux;
    CComPtr<IBaseFilter>       pNullRenderer;
    CComPtr<IBaseFilter>       pFileWriter;
    CComPtr<IPin>              pOutPin;

    // Convert target filename to a wide character string
#ifndef _UNICODE
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR) szTarget, -1, 
                        wszTargetFile, NUMELMS(wszTargetFile));
#else
    wcsncpy(wszTargetFile, szTarget, NUMELMS(wszTargetFile));

#endif
    
    // Create an empty DirectShow filter graph
    hr = CreateFilterGraph(&pGraph);
    ON_FAIL("Couldn't create filter graph!", hr)
    
    // Convert the source file into WCHARs for DirectShow
#ifndef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szSource, -1, wszSourceFile, NUMELMS(wszSourceFile));
#else
        wcsncpy(wszSourceFile, szSource, NUMELMS(wszSourceFile));
#endif

    // Add the file source filter
    hr = pGraph->AddSourceFilter(wszSourceFile, L"Source Filter", &pSourceFilter);
    ON_FAIL("Failed to add source filter!", hr);
	
    // Add the AVI Splitter. 
    // Note: For the sake of simplicity this sample only handles uncompressed AVI 
    // files as input. It does not handle DV-AVI files or AVI files with compressed video
    // content. The uncompressed video can use any color space (RGBxx, YUY2, UYUV, etc)
    // that the WMV codec can handle.
    hr = AddNewFilter(CLSID_AviSplitter, &pAviSplitter, pGraph, L"AviSplitter");
    ON_FAIL("Failed to create and add AVI Splitter filter!", hr) 
     
    // Connect Source Filter to AVI Splitter
    hr = ConnectFilters(pSourceFilter, pAviSplitter, pGraph, NULL);
    ON_FAIL("Failed to connect source filter to avi splitter!",  hr)
    
    // Add the DMO Wrapper filter
    hr = AddNewFilter(CLSID_DMOWrapperFilter, &pDMOWrapper, pGraph, L"DMO Wrapper");
    ON_FAIL("Failed to create and add DMO Wrapper filter!",  hr)
  
    // Init the DMO wrapper with the WMV9 DMO
    CComQIPtr<IDMOWrapperFilter, &IID_IDMOWrapperFilter> pDMOWrapperFilter(pDMOWrapper);
    ON_QI_FAIL("Failed to QI for IDMOWrapperFilter!", pDMOWrapperFilter)

    hr = pDMOWrapperFilter->Init(CLSID_CWMVEncMediaObject2, CLSID_VideoCompressorCategory);
    ON_FAIL("Failed to Init DMOWrapper!",hr)

    //Get the pointer to the DMO
    CComQIPtr<IMediaObject, &IID_IMediaObject> pDMO(pDMOWrapper);
    ON_QI_FAIL("Failed to QI for IMediaObject!", pDMO)

    //To configure the WMV codec we need the input media type
    // The easiest way to get this is to connect the AVI Splitter to a dummy filter (the Null Renderer) downstream
    // and then get the ConnectionMediaType. 
    AM_MEDIA_TYPE mtIn, mtOut;
    mtIn.majortype = MEDIATYPE_Video; //set this for the call to "Connect Filters"
    mtIn.cbFormat = 0;

    hr = GetInputMediaType(pGraph, pAviSplitter, mtIn); //S_FALSE is ok here
    ON_FAIL("Failed to GetInputMediaType!",hr)    

    // Connect AVI Splitter video pin to DMO Wrapper
    hr = ConnectFilters(pAviSplitter, pDMOWrapper, pGraph, &mtIn);
    ON_FAIL("Failed to connect AVI Splitter and DMO Wrapper filters!", hr)
  
    hr = ConfigureDMO(pDMO, &mtIn, &mtOut);
    ON_FAIL("Failed to configure DMO!", hr)

    //Get Wrapper's output pin
    hr = GetPin( pDMOWrapper, PINDIR_OUTPUT, &pOutPin );
    ON_FAIL("Failed to get wrapper's output pin!", hr)

    //Set the output media type
    CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> pIAMStreamConfig( pOutPin );    
    ON_QI_FAIL("Failed to QI for IAMStreamConfig!", pIAMStreamConfig)

    //Set the media type
    hr = pIAMStreamConfig->SetFormat( &mtOut );
    ON_FAIL("Failed to set wrapper's output type!", hr)

    //Create the AVI Mux but don't connect it to the DMO Wrapper yet
    hr = AddNewFilter(CLSID_AviDest, &pAviMux, pGraph, L"AVI Mux");
    ON_FAIL("Failed to create AVI Mux filter!",  hr)

    //Add the File Writer
    hr = AddNewFilter(CLSID_FileWriter, &pFileWriter, pGraph, L"File Writer");
    ON_FAIL("Failed to create File Writer filter!", hr)

    // Get a file sink filter interface from the File Writer filter
    // and set the output file name
    CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pFileSink (pFileWriter);
    ON_QI_FAIL("Failed to create QI IFileSinkFilter!", pFileSink)

    hr = pFileSink->SetFileName(wszTargetFile, NULL);
    ON_FAIL("Failed to set target filename!", hr)

    // Now we can connect the AVI Mux to the DMO Wrapper.
    hr = ConnectFilters( pDMOWrapper, pAviMux, pGraph, &mtOut);
    ON_FAIL("Failed to connect mux to dmo wrapper video!", hr)


    // Connect the AVI Mux to the File Writer.
    hr = ConnectFilters(pAviMux, pFileWriter, pGraph, NULL);
    ON_FAIL("Failed to connect mux to file writer!", hr)

#ifdef HAS_PCM

    // Connect the AVI Mux to the AVI SPlitter Audio Out
    // We can't use "ConnectFilters" since the first AVI Mux pin is already connected
    CComPtr<IPin> pSplitterAudioOut;
    CComPtr<IPin> pMuxAudioIn;

    hr = GetNextUnconnectedPin(pAviSplitter, PINDIR_OUTPUT, &pSplitterAudioOut);
    ON_FAIL("Failed to get next avi splitter unconnected pin!", hr)

    hr = GetNextUnconnectedPin(pAviMux, PINDIR_INPUT, &pMuxAudioIn);
    ON_FAIL("Failed to get next avi mux unconnected pin!", hr)

    hr = pGraph->Connect(pSplitterAudioOut, pMuxAudioIn);
    ON_FAIL("Failed to connect mux to dmo wrapper uedio!", hr)

#endif

    // Set sync source to NULL to encode as fast as possible
    SetNoClock(pGraph);
    DbgLog((LOG_TRACE, 3, _T("\nCopying [%ls] to [%ls]\n"), wszSourceFile, wszTargetFile));

    // Now we are ready to run the filter graph and start encoding. First we need the IMediaControl interface.
    CComQIPtr<IMediaControl, &IID_IMediaControl> pMC(pGraph);
    ON_QI_FAIL("Failed to QI for IMediaControl!", pMC)
    
    hr = pMC->Run();
    ON_FAIL("Graph was built but could not run!",  hr)
 
    // Wait for the event signalling that we have reached the end of the input file. We listen for the
    // EC_COMPLETE event here rather than in the app's message loop in order to keep the order of operations
    // as straightforward as possible. The downside is that we cannot stop or pause the graph once it starts. 
    int nEvent = WaitForCompletion(g_hwnd, pGraph);

    

    // Stop the graph. 
    hr = pMC->Stop();
    ON_FAIL("Failed to stop filter graph!", hr)
    
 
    //Clean up
    FreeMediaType(mtIn);
    FreeMediaType(mtOut);

    return hr;
}

//------------------------------------------------------------------------------
// Name: OpenFileDialog()
// Desc: Open the File Open dialog box in order to select a source file, profile file, 
//       and target file name.
//------------------------------------------------------------------------------

BOOL OpenFileDialog(HWND hWnd)
{
    OPENFILENAME ofn;
    _TCHAR        szFileName[_MAX_PATH];
    _TCHAR        szMediaFile[] = "Media Files\0*.avi;*.mpg;*.wav;*.wmv; *.mp3\0\0";
    _TCHAR        szPRXFile[] = "PRX Files\0*.prx";

    szFileName[0] = 0;
  
    _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = hWnd ;
    ofn.lpstrFilter = szMediaFile;
	ofn.lpstrTitle = "Select file to be converted";
    
    ofn.nFilterIndex = 0 ;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) ;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0 ;
    ofn.lpstrInitialDir = "c:\\"; 
    ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST ;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hWnd, IDC_SOURCEFILE, szFileName);
	return TRUE;
    } else {
	return FALSE;
    }
}

//------------------------------------------------------------------------------
// Name: CreateFilterGraph()
// Desc: Create a DirectShow filter graph.
//------------------------------------------------------------------------------
HRESULT CreateFilterGraph(IGraphBuilder **pGraph)
{
    HRESULT hr;

    if (!pGraph)
        return E_POINTER;

    hr = CoCreateInstance(CLSID_FilterGraph, // get the graph object
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder,
                          (void **) pGraph);

    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("CreateFilterGraph: Failed to create graph!  hr=0x%x\n"), hr));
        *pGraph = NULL;
        return hr;
    }

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: CreateFilter()
// Desc: Create a DirectShow filter .
//------------------------------------------------------------------------------
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{
    HRESULT hr;

    if (!ppFilter)
        return E_POINTER;

    hr = CoCreateInstance(clsid,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter,
                          (void **) ppFilter);

    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("CreateFilter: Failed to create filter!  hr=0x%x\n"), hr));
        *ppFilter = NULL;
        return hr;
    }

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: AddNewFilter()
// Desc: Create a DirectShow filter and add it to the filter graph.
//------------------------------------------------------------------------------
HRESULT AddNewFilter(REFCLSID clsid, IBaseFilter **ppFilter, IGraphBuilder* pGB, LPCWSTR pFilterName)
{
    HRESULT hr;

    if (!ppFilter)
        return E_POINTER;

    hr = CoCreateInstance(clsid,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter,
                          (void **) ppFilter);

    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 3, _T("CreateFilter: Failed to create filter!  hr=0x%x\n"), hr));
        *ppFilter = NULL;
        return hr;
    }

    hr = pGB->AddFilter(*ppFilter, pFilterName);
    ON_FAIL("CreateFilter: Failed to add filter!", hr)

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: SetNoClock()
// Desc: Prevents an unnecessary clock from being created.
// This speeds up the copying process, since the renderer won't wait
// for the proper time to render a sample; instead, the data will
// be processed as fast as possible.
//------------------------------------------------------------------------------
HRESULT SetNoClock(IFilterGraph *pGraph)
{
    if (!pGraph)
        return E_POINTER;

    CComPtr<IMediaFilter> pFilter;
    HRESULT hr = pGraph->QueryInterface(IID_IMediaFilter, (void **) &pFilter);

    if(SUCCEEDED(hr))
    {
        // Set to "no clock"
        hr = pFilter->SetSyncSource(NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 3, _T("SetNoClock: Failed to set sync source!  hr=0x%x\n"), hr));
        }
    }
    else
    {
        DbgLog((LOG_TRACE, 3, _T("SetNoClock: Failed to QI for media filter!  hr=0x%x\n"), hr));
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: OutputMsg()
// Desc: Displays status messages in the user interface output window
//------------------------------------------------------------------------------

void OutputMsg(_TCHAR* msg)
{

	int nInputLen = (int) _tcslen(msg);
	g_OutputStringLen += nInputLen;

    if ( g_nOutputStringBufferSize > g_OutputStringLen + 1)
    {
       _tcsncat(szOutputWindow, msg, (size_t) nInputLen);
    }
    
 
	HWND hDesc = GetDlgItem(g_hwnd, IDC_DESC);
    SetWindowText(hDesc, szOutputWindow);
    
}


//------------------------------------------------------------------------------
// Name: WaitForCompletion()
// Desc: Waits for a media event that signifies completion or cancellation
//       of a task.
//------------------------------------------------------------------------------
LONG WaitForCompletion(HWND g_hwnd, IGraphBuilder *pGraph )
{
    HRESULT hr;
    LONG levCode = 0;
    CComPtr <IMediaEvent> pME;

    if (!pGraph)
        return -1;
        
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **) &pME);
    if (SUCCEEDED(hr))
    {
        OutputMsg(_T("Waiting for completion. This could take several minutes, depending on file size and codec settings.\r\n"));
        DbgLog((LOG_TRACE, 3, _T("Waiting for completion...\n  This could take several minutes, ")
                 _T("depending on file size and codec settings.\n")));
        HANDLE hEvent;
        
        hr = pME->GetEventHandle((OAEVENT *)&hEvent);
        if(SUCCEEDED(hr)) 
        {
            // Wait for completion and dispatch messages for any windows
            // created on our thread.
            for(;;)
            {
                while(MsgWaitForMultipleObjects(
                    1,
                    &hEvent,
                    FALSE,
                    INFINITE,
                    QS_ALLINPUT) != WAIT_OBJECT_0)
                {
                    MSG Message;

                    while (PeekMessage(&Message, NULL, 0, 0, TRUE))
                    {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }
                }

                // Event signaled. See if we're done.
                LONG_PTR lp1, lp2;
                
                if(pME->GetEvent(&levCode, &lp1, &lp2, 0) == S_OK)
                {
                    pME->FreeEventParams(levCode, lp1, lp2);
                
                    if(EC_COMPLETE == levCode)
                    {
                        OutputMsg(_T("Encoding complete!\r\n"));
                        // Display received event information
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_COMPLETE.\n")));
                        }                            
                        break;
                    }
                    else if(EC_ERRORABORT == levCode)
                    {
                        OutputMsg(_T("Error abort!\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_ERRORABORT.\n")));
                        }                            
                        break;
                    }
                    else if(EC_USERABORT == levCode)
                    {
                        OutputMsg(_T("User Abort\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received EC_USERABORT.\n")));
                        }
                        break;
                    }
                    
                    else
                    {   
                        OutputMsg(_T("Received some unknown event!\r\n"));
                        if (g_fVerbose)
                        {
                            DbgLog((LOG_TRACE, 3, _T("WaitForCompletion: Received event %d.\n"), levCode));
                        }
                    }
                }
            }
        }
        else
        {
            OutputMsg(_T("Unexpected Failure!\r\n"));
            DbgLog((LOG_TRACE, 3, _T("Unexpected failure (GetEventHandle failed)...\n")));
        }
    }        
    else
        DbgLog((LOG_TRACE, 3, _T("QI failed for IMediaEvent interface!\n")));

    return levCode;
}

BOOL CALLBACK Dlg_Main( HWND hDlg,  UINT msg, WPARAM wParam, LPARAM lParam)
 
{
    g_hwnd = hDlg;
    HWND hChildWindow = 0;
    HWND hProfile = 0;
    HWND hComplexity = 0;
    HRESULT hr = S_OK;
	int nSourceLength = 0;
    VideoEncParams vep1;

	switch(msg)
	{
		case WM_INITDIALOG:
                
			SetEncodingDefaults(hDlg);
                
			break;

		case WM_COMMAND:
			
			switch(wParam)
			{
            
			
                case IDC_BTN_SOURCEFILE:
                    OpenFileDialog(hDlg);
                    GetDlgItemText(hDlg, IDC_SOURCEFILE, szSource, _MAX_PATH);
                    nSourceLength = (int) strlen(szSource);
                    ZeroMemory(szTarget, _MAX_PATH);
                    if(nSourceLength > 5)
                    {
                        strncpy(szTarget, szSource, nSourceLength - 4);
                        strcat(szTarget, ".avi"); 
                        SetDlgItemText(hDlg, IDC_TARGETFILE, szTarget);
                    }
                    break;

                case IDC_BTN_PRXFILE:
                    OpenFileDialog(hDlg);
                    break;

                case IDC_CBR:
                    g_VideoEncodingParams.fIsVBR = FALSE;
                    //Disable VBR Quality window
                    hChildWindow = GetDlgItem( hDlg, IDC_VBRQUALITY);
                    EnableWindow( hChildWindow, FALSE );

                    //Ensable CBR Quality (Crispness) window
                    hChildWindow = GetDlgItem( hDlg, IDC_QUALITY);
                    EnableWindow( hChildWindow, TRUE );

                    //Enable Buffer Window and Bitrate as they do apply to Quality VBR
                    hChildWindow = GetDlgItem( hDlg, IDC_BITRATE);
                    EnableWindow( hChildWindow, TRUE );
                    hChildWindow = GetDlgItem( hDlg, IDC_BUFFERDELAY);
                    EnableWindow( hChildWindow, TRUE );
                    break;

                case IDC_QUALITY_VBR:

                    g_VideoEncodingParams.fIsVBR = TRUE;
                    //Enable VBR Quality window
                    hChildWindow = GetDlgItem( hDlg, IDC_VBRQUALITY);
                    EnableWindow( hChildWindow, TRUE );
                    //Disable Crispness window
                    //Disable VBR Quality window
                    hChildWindow = GetDlgItem( hDlg, IDC_QUALITY);
                    EnableWindow( hChildWindow, FALSE );

                    //Disable Buffer Window and Bitrate as they don't apply to Quality VBR
                    hChildWindow = GetDlgItem( hDlg, IDC_BITRATE);
                    EnableWindow( hChildWindow, FALSE );
                    hChildWindow = GetDlgItem( hDlg, IDC_BUFFERDELAY);
                    EnableWindow( hChildWindow, FALSE );

                    break;

                

                case ID_GO:

                    ZeroMemory(szOutputWindow, g_nOutputStringBufferSize);
                    GetDlgItemText(hDlg, IDC_SOURCEFILE, szSource, _MAX_PATH);
                    GetDlgItemText(hDlg, IDC_TARGETFILE, szTarget, _MAX_PATH);
                    if(strcmp(szSource, szTarget) == 0)
                    {
                        MessageBox(hDlg, "Target file must be different from source file", NULL, MB_OK);
                        break;
                    }
                    
                    CopyMemory((VideoEncParams*)&vep1, &g_VideoEncodingParams, sizeof(VideoEncParams));
                    
                    g_VideoEncodingParams.fFrameRate = GetDlgFloat(hDlg, IDC_FRAMERATE);
                        
                    
                    //only WMV3 is demonstrated
                    g_VideoEncodingParams.dwTag = WMCFOURCC_WMV3;
                    
                    // Get the user-selected codec settings and enforce some limits                  
                    g_VideoEncodingParams.nBitrate = GetDlgItemInt(hDlg, IDC_BITRATE, NULL, FALSE);
                    g_VideoEncodingParams.nBitrate > 24000000 ? 24000000 : g_VideoEncodingParams.nBitrate;
                    // zero bitrate means let codec decide

                    // This should be typically 3000 - 5000. A 128 second buffer is the absolute limit but is
                    // absurdly high for practical encoding purposes.
                    g_VideoEncodingParams.nBufferDelay = GetDlgItemInt(hDlg, IDC_BUFFERDELAY, NULL, FALSE);
                    g_VideoEncodingParams.nBufferDelay < 1000 ? 1000 : g_VideoEncodingParams.nBufferDelay;
                    g_VideoEncodingParams.nBufferDelay > 128000 ? 128000 : g_VideoEncodingParams.nBufferDelay;
 
                    g_VideoEncodingParams.nKeyDist = GetDlgItemInt(hDlg, IDC_MAXKEYDISTANCE, NULL, FALSE);
                    g_VideoEncodingParams.nKeyDist > 100 ? 100 : g_VideoEncodingParams.nKeyDist;
                    g_VideoEncodingParams.nKeyDist < 1 ? 1 : g_VideoEncodingParams.nKeyDist;

                    hProfile = GetDlgItem(hDlg, IDC_PROFILES);
                    g_VideoEncodingParams.nProfile = (int) SendMessage(hProfile, LB_GETCURSEL, 0 , 0);

                    hComplexity = GetDlgItem(hDlg, IDC_COMPLEXITY);
                    g_VideoEncodingParams.nComplexity = (int) SendMessage(hComplexity, LB_GETCURSEL, 0, 0);
             
                    g_VideoEncodingParams.nQuality = GetDlgItemInt(hDlg, IDC_QUALITY, NULL, FALSE);
                    g_VideoEncodingParams.nQuality < 0 ? 0 : g_VideoEncodingParams.nQuality;
                    g_VideoEncodingParams.nQuality > 100 ? 100 : g_VideoEncodingParams.nQuality;
                    
                    //Ignored by codec in CBR mode
                    g_VideoEncodingParams.nVBRQuality = GetDlgItemInt(hDlg, IDC_VBRQUALITY, NULL, FALSE);

                    if(szSource[0] == '\0')
                    {
                        MessageBox(g_hwnd, "Please enter a source file name", NULL, MB_OK);
                        break;
                    }
                    if(szTarget[0] == '\0')
                    {
                        MessageBox(g_hwnd, "Please enter a target file name", NULL, MB_OK);
                        break;
                    }

                    
                    hr = MakeWMVAVIFile(szSource, szTarget, szProfile);
                    if(FAILED(hr))
                    {
                        OutputMsg(_T("Encoding session failed.\r\n"));
                    }
                    
                    DbgLog((LOG_TRACE, 3, _T("Completed MakeWMVAVIFile\n")));
                    
					break;
                    
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
                    DbgLog((LOG_TRACE, 3, _T("End dialog\n")));
                    
					break;
			
			}
		break;

		default:
		break;
	
	}

    return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	hInst = hInstance;
	CoInitialize(NULL);

	// Dshow debug initialization
	DbgInitialise(hInst);
	DbgSetModuleLevel(LOG_TRACE | LOG_ERROR, 3);
    
   	// This will loop until the Exit button is hit
	DialogBox(hInst, MAKEINTRESOURCE(IDD_WMVINAVI), NULL, (DLGPROC)Dlg_Main);

    // Clean up
	CoUninitialize();
 	DbgTerminate(); //causes access violation
	
    return 0;
}




HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (PinDir == PinDirThis)
        {
            *ppPin = pPin.Detach();
            return S_OK;
        }
        pPin.Release();
    }
    
    return E_FAIL;
}

//Connect two filters using either Connect or ConnectDirect. ConnectDirect is used if a majortype is specified in pmt.
HRESULT ConnectFilters(IBaseFilter* pUpstream, IBaseFilter* pDownstream, IGraphBuilder* pGraph, AM_MEDIA_TYPE* pmt)
{
    HRESULT hr;
    CComPtr<IPin> pOutputPin;
    CComPtr<IPin> pInputPin;
   
    //caller specifies a media type for two situations: if we need to connect direct, and if we have more than one output pin
    // and need to distinguish. Of course our implementation won't work if we have two+ video or two+ uedio outputs. That is left as an exercise for the reader.

    if(pmt)
        hr = GetPinByMajorType(pUpstream, PINDIR_OUTPUT, pmt->majortype, &pOutputPin);
    else
        hr = GetPin(pUpstream, PINDIR_OUTPUT, &pOutputPin);
    ON_FAIL("Failed to get output pin!", hr)
   
    // NOTE: this will only work for the first pin on a filter
    hr = GetPin(pDownstream, PINDIR_INPUT, &pInputPin);
    CComPtr<IPin> pSplitterOutPin;
	ON_FAIL("Failed to get avi mux vid input pin!", hr)
    
    // cbFormat will be zero if we have passed in a dummy mt with only the major type set
    // to use in GetPinByMajorType. We don't want to use such an mt for ConnectDirect as it will fail
    if(NULL == pmt || pmt->cbFormat == 0)
    {
        hr = pGraph->Connect(pOutputPin, pInputPin);
        ON_FAIL("Failed to Connect encoder to mux!", hr)
    }
    else
    {
        hr = pGraph->ConnectDirect(pOutputPin, pInputPin, pmt);//BUGBUG pmt = 0
        ON_FAIL("Failed to ConnectDirect encoder to mux!", hr)
    }

    return hr;
}

// Determine the media type on the filter upstream from the DMO by connecting the upstream filter
// to the Null Renderer temporarily and then seeing what media type was used for that connection.
HRESULT GetInputMediaType(IGraphBuilder* pGB, IBaseFilter* pSplitter, AM_MEDIA_TYPE& mt)
{
    HRESULT hr;

    CComPtr<IBaseFilter> pDummyFilter;
	CComPtr<IPin> pSplitterOutPin;

    hr = AddNewFilter(CLSID_NullRenderer, &pDummyFilter, pGB, L"Dummy Filter");
    ON_FAIL("Failed to create dummy filter!", hr)

    hr = ConnectFilters(pSplitter, pDummyFilter, pGB, &mt);
    ON_FAIL("Failed to connect dummy filter!", hr)
    
        hr = GetPinByMajorType(pSplitter, PINDIR_OUTPUT, mt.majortype, &pSplitterOutPin);
    ON_FAIL("Failed to get splitter out pin!", hr)

    hr = pSplitterOutPin->ConnectionMediaType(&mt);
    ON_FAIL("Failed to get connection media type!", hr)

    hr = pSplitterOutPin->Disconnect();
    ON_FAIL("Failed to disconnect splitter out pin!", hr)

    //now disconnect and remove dummy filter
    hr = pGB->RemoveFilter(pDummyFilter);
    ON_FAIL("Failed to remove dummy filter!", hr)
    return hr;
}


HRESULT ConfigureDMO(IMediaObject* pDMO, AM_MEDIA_TYPE* pmtIn, AM_MEDIA_TYPE* pmtOut)
{
    HRESULT hr;

    if ( FORMAT_VideoInfo  == pmtIn->formattype  || FORMAT_VideoInfo2 == pmtIn->formattype )
    {
        hr = InitializeVideoEncoder( pmtIn, &g_VideoEncodingParams, pDMO, pmtOut );
    }
    else 
    {
        FreeMediaType( *pmtIn );
        memset( pmtIn, 0, sizeof( AM_MEDIA_TYPE ) );
        hr = E_FAIL;
    }

    
    return hr;
}

HRESULT GetPinByMajorType(IBaseFilter *pFilter, PIN_DIRECTION PinDir, GUID majortype, IPin** ppPin)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    ON_FAIL("Failed to EnumPins in GetPinByMajorType!", hr)

    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        AM_MEDIA_TYPE mt;
		
        // Try this first as it is easy
        hr = pPin->ConnectionMediaType(&mt);
        if(SUCCEEDED(hr))
        {
            if (PinDir == PinDirThis && IsEqualGUID(majortype, mt.majortype))
            {
                *ppPin = pPin.Detach();
                CoTaskMemFree(mt.pbFormat);
                return S_OK;
            }

		    CoTaskMemFree(mt.pbFormat); 
        }
        else
        {
            if(hr == VFW_E_NOT_CONNECTED)
            {
                CComPtr<IEnumMediaTypes> pEnumMediaTypes;
                AM_MEDIA_TYPE* pMT;
                pPin->EnumMediaTypes(&pEnumMediaTypes);
                while(pEnumMediaTypes->Next(1, &pMT, 0) == S_OK)
                {
                    if(pMT->majortype == majortype)
                    {
                        *ppPin = pPin.Detach();
                        FreeMediaType(*pMT);
                        return S_OK;

                    }
                    FreeMediaType(*pMT);
                } // end while

            } // end if VFW_E_NOT_CONNECTED
        } //end else
        pPin.Release();
    } // while while pPin
    
    return E_FAIL;  
}

HRESULT GetNextUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    ON_FAIL("Failed to EnumPins in GetPinByMajorType!", hr)

    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
		CComPtr<IPin> pTempPin;
        // Try this first as it is easy
        hr = pPin->ConnectedTo(&pTempPin);
        if(SUCCEEDED(hr))
        {
            pPin.Release();
            continue;
        }
        else
        {
            if(hr == VFW_E_NOT_CONNECTED)
            {

               *ppPin = pPin.Detach();
               return S_OK;
  
            } // end if VFW_E_NOT_CONNECTED
        } //end else
        pPin.Release();
    } // while while pPin
    
    return E_FAIL;  
}


void SetEncodingDefaults(HWND hDlg)
{
    HWND hcb;

    DWORD ComplexityLive = 0;
    DWORD ComplexityOffline = 0;
    DWORD ComplexityMax = 0;
    char  ComplexityString[15];
    HRESULT hr = S_OK;

    g_VideoEncodingParams.fFrameRate = 29.97F;
    
    // Other codecs not supported in this sample
    g_VideoEncodingParams.dwTag = WMCFOURCC_WMV3;

    g_VideoEncodingParams.fIsVBR = FALSE;
    g_VideoEncodingParams.nBitrate = 0;
    g_VideoEncodingParams.nBufferDelay = 5000;
    g_VideoEncodingParams.nKeyDist = 8;
    g_VideoEncodingParams.nProfile = P_MAIN;
    g_VideoEncodingParams.nQuality = 75;
    g_VideoEncodingParams.nVBRQuality = 98;
    
    hcb = GetDlgItem(hDlg, IDC_CBR);
    SendMessage(hcb, BM_SETCHECK, BST_CHECKED, NULL);

    //Disable while CBR is checked
    hcb = GetDlgItem( hDlg, IDC_VBRQUALITY);
    EnableWindow( hcb, FALSE );

    SetDlgFloat(hDlg, IDC_FRAMERATE, g_VideoEncodingParams.fFrameRate);
    SetDlgItemInt(hDlg, IDC_BUFFERDELAY, g_VideoEncodingParams.nBufferDelay, TRUE);

    SetDlgItemInt(hDlg, IDC_BITRATE, g_VideoEncodingParams.nBitrate, TRUE);

    SetDlgItemInt(hDlg, IDC_MAXKEYDISTANCE, g_VideoEncodingParams.nKeyDist, TRUE);

    SetDlgItemInt(hDlg, IDC_QUALITY, g_VideoEncodingParams.nQuality, TRUE);
    SetDlgItemInt(hDlg, IDC_VBRQUALITY, g_VideoEncodingParams.nVBRQuality, TRUE);

    //Add profiles and set default selection
    hcb = GetDlgItem(hDlg, IDC_PROFILES);
    SendMessage(hcb, LB_ADDSTRING, NULL, (LPARAM) "Main");
    SendMessage(hcb, LB_ADDSTRING, NULL, (LPARAM) "Simple");
    SendMessage(hcb, LB_ADDSTRING, NULL, (LPARAM) "Complex");
    SendMessage(hcb, LB_SETCURSEL, g_VideoEncodingParams.nProfile, NULL);
    
    // Add complexity levels.
    hcb = GetDlgItem(hDlg, IDC_COMPLEXITY);

    // Get the complexity levels for the codec.
    hr = GetComplexitySettings(&ComplexityLive, 
                               &ComplexityOffline, 
                               &ComplexityMax);

    // Loop through the available complexity settings, 
    //  adding an entry for each.
    for(DWORD index = 0; index <= ComplexityMax; index++)
    {
        // Assemble the complexity list entry string 
        if(index == ComplexityLive)
            sprintf(ComplexityString, "%d (Live)", index);
        else if(index == ComplexityOffline)
            sprintf(ComplexityString, "%d (Offline)", index);
        else if(index == ComplexityMax)
            sprintf(ComplexityString, "%d (Max)", index);
        else
            sprintf(ComplexityString, "%d", index);
            
        // Add the string to the list box.        
        SendMessage(hcb, LB_ADDSTRING, NULL, (LPARAM) ComplexityString);
    }
    SendMessage(hcb, LB_SETCURSEL, ComplexityOffline, NULL);

}

//-----------------------------------------------------------------------------
// Name:    GetDlgFloat
// Desc:    Utility to extract a floating point number from a dialog
//          Specifically used to read frame rates such as 29.97
//-----------------------------------------------------------------------------

float GetDlgFloat(HWND hDlg, int dlgItem)

{

      float value;

      char  Text[25] = {'\0'};
      GetDlgItemText(hDlg, dlgItem, Text, 24);
      sscanf(Text,"%f",&value);
      return value;

}

 

//-----------------------------------------------------------------------------

// Name:    SetDlgFloat

// Desc:    Utility to load a floating point number into a dialog

//-----------------------------------------------------------------------------

void  SetDlgFloat(HWND hDlg, int dlgItem, float value)

{

      char  floatText[25] = {'\0'};

      sprintf(floatText,"%1.2f",value);
      SetDlgItemText(hDlg, dlgItem, floatText);

}

// Get the encoder complexity values for the codec.
HRESULT GetComplexitySettings(DWORD* pLive, DWORD* pOffline, DWORD* pMax)
{
    HRESULT hr = S_OK;
    
    CComPtr<IMediaObject>  pDMO;
    CComPtr<IWMCodecProps> pCodecProps;

    DWORD cbValue = sizeof(DWORD);
    WMT_PROP_DATATYPE dataType;
    

    // Create a video encoder DMO.
    hr = CoCreateInstance(CLSID_CWMVEncMediaObject2,
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IMediaObject, 
                          (void**)&pDMO);
    ON_FAIL("Could not create the encoder DMO.", hr);

    // Get the codec properties interface.
    hr = pDMO->QueryInterface(IID_IWMCodecProps, (void**)&pCodecProps);
    ON_FAIL("Could not get the codec props interface.", hr);

    // Get the setting for live encoding.
    hr = pCodecProps->GetCodecProp(WMCFOURCC_WMV3, 
                                   g_wszWMVCComplexityExLive, 
                                   &dataType, 
                                   (BYTE*)pLive, 
                                   &cbValue);
    ON_FAIL("Could not get the live complexity setting.", hr);

    // Get the setting for offline encoding.
    hr = pCodecProps->GetCodecProp(WMCFOURCC_WMV3, 
                                   g_wszWMVCComplexityExOffline, 
                                   &dataType, 
                                   (BYTE*)pOffline, 
                                   &cbValue);
    ON_FAIL("Could not get the offline complexity setting.", hr);

    // Get the maximum complexity setting.
    hr = pCodecProps->GetCodecProp(WMCFOURCC_WMV3, 
                                   g_wszWMVCComplexityExMax, 
                                   &dataType, 
                                   (BYTE*)pMax, 
                                   &cbValue);
    ON_FAIL("Could not get the maximum complexity setting.", hr);

    return hr;

}

