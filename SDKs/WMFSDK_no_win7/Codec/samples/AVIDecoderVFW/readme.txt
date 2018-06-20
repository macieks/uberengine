========================================
CONSOLE APPLICATION: AVIDecoderVFW
========================================

This sample uses the Video For Windows API to manage input and output AVI files. The user can specify the input/outpt files with command-line arguments.

Usage
-----

AVIDecoderVFW -input source -output destination [params]

Where source is the input AVI file name and destination is the output AVI file name. The optional parameters can include any in the following table.
Parameter Description 
-audio_only  decodes only the compressed audio stream
-video_only  decodes only the compressed video stream

Build instructions
------------------
Install DirectX 9.0 SDK (the sample assumes the DX SDK root is c:\DXSDK or c:\DX90SDK)
Build AVIDecoderVFW.dsp in this folder

Please post questions or comments about this sample to microsoft.public.windowsmedia.sdk.
