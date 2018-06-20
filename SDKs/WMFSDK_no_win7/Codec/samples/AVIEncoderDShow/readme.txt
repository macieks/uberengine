============================================
AVIEncoderDShow
============================================

This Windows dialog-based application demonstrates how to use DirectShow and the Windows Media Video Encoder DMO to place Windows Media Video content into an AVI file. The concepts shown in this application can be applied to any other file format provided that you have filters to multiplex the data streams. The sample is hard coded to handle as input only uncompressed video streams. Most YUV and RGB pixel formats will be accepted. Because of issues with audio-video synchronization when Windows Media Audio is used in AVI files, the sample does not demonstrate the use of the WMA Encoder. However, it does show how to connect an uncompressed PCM audio stream directly from the AVI Splitter to the AVI Multiplexer. If you wish to include PCM audio in your output file, simply define "HAS_PCM" in main.cpp before compiling.

Build instructions:
Install DirectX 9.0 SDK (the sample assumes the DX SDK root is c:\DXSDK or c:\DX90SDK)
Build the debug DirectShow base class library (strmbasd.lib) in c:\DX[90]SDK\Samples\C++\DirectShow\BaseClasses.
Build AVIEncoderDShow.dsp in this folder

Please post questions or comments about this sample to microsoft.public.windowsmedia.sdk.
