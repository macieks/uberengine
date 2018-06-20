========================================================================
       WIN32 Console Application : DRMUpdate 
========================================================================

Demonstrates how to perform individualization.

USAGE:

DRMUpdate <ReportFile> [-force]

    ReportFile = Name of the outpout report file. If this file exists, 
                 it will be overwritten.
    -force     = If used, this parameter forces individualization. If
                 not used, the DRM component will determine whether
                 individualization is required.

HOW TO BUILD:

  In order to build the sample executable, open the project file 
DRMUpdate.sln in Visual C++ and build the project.


IMPORTANT INTERFACES DEMONSTRATED IN THIS SAMPLE:

    IWMDRMSecurity