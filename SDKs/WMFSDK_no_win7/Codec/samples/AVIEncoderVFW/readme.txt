========================================
CONSOLE APPLICATION: AVIEncoderVFW
========================================

This sample uses the Video For Windows API to manage input and output AVI files. The user can specify encoding parameters with command-line arguments, or have the application figure defaults.

Usage
-----

AVIEncoderVFW -input source -output destination [params]

Where source is the input AVI file name and destination is the output AVI file name. The optional parameters can include any in the following table. Other parameters are accepted by the application, but represent features not supported in this release.

Parameter Description 
-v_mode The encoding mode of the video stream. Set to one of the following strings: 
"1_pass_cbr" 
"2_pass_cbr" 
"1_pass_vbr" 
"2_pass_vbr" 
"2_pass_vbr_constrained" 
 
-v_bitrate       The bit rate to use for the video stream. 
-v_framerate     The frame rate to use for the video stream. 
-v_keydist       The maximum key-frame distance for the video stream (in miliseconds). 
-v_buffer        The buffer window, in milliseconds, of the video stream. 
-v_quality       The crispness value to use for the video stream. 
-v_vbrquality    The VBR quality value to use for the video stream. Use only for 1-pass VBR encoding. 
-v_peakbuffer    The peak buffer-window value for peak-constrained 2-pass VBR encoding. 
-v_peakbitrate   The peak bit-rate value for peak constrained 2-pass VBR encoding. 
-v_profile       The device conformance template level to which the video content should conform. Set to "main", "simple", or "complex". 
-v_complexity    The encoder complexity for the video stream. 
-v_forcekey      The frame number to be forced key frame. 

-a_codec	 The audio codec to be used (only WMSP supported)
-a_bitrate	 Audio bitrate, 4000 or 20000 bps, default: 20000
-a_type	         Audio codec mode, speech or auto,  default: speech

Build instructions
------------------
Install DirectX 9.0 SDK (the sample assumes the DX SDK root is c:\DXSDK or c:\DX90SDK)
Build AVIEncoderVFW.dsp in this folder

Please post questions or comments about this sample to microsoft.public.windowsmedia.sdk.
