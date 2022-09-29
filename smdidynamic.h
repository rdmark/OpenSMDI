/*
OpenSMDI - A retargetable shared library for SMDI transfers from/to a sampler
Copyright (C) 1999,2000 Christian Nowak

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.


Contact the original author via paper mail:
Christian Nowak
Groendelle 9
42555 Velbert
Germany

e-Mail: chnowak@web.de

*/


#ifndef _SMDIDYNAMIC_H
#define _SMDIDYNAMIC_H

#include "smdi.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long (*SMDI_GetVersion) (void) = NULL;
unsigned char (* SMDI_Init) (void) = NULL;
void (*SMDI_GetDeviceInfo) ( unsigned char, unsigned char, SCSI_DevInfo * ) = NULL;
unsigned long (*SMDI_SampleHeaderRequest) ( unsigned char,
                                            unsigned char, 
                                            unsigned long, 
                                            SMDI_SampleHeader * ) = NULL;
unsigned long (*SMDI_SendSampleHeader) ( unsigned char,
                                         unsigned char, 
                                         unsigned long, 
                                         SMDI_SampleHeader *, 
                                         unsigned long * ) = NULL;
unsigned long (*SMDI_SendBeginSampleTransfer) ( unsigned char,
                                                unsigned char, 
                                                unsigned long, 
                                                void * ) = NULL;

unsigned long (*SMDI_SendDataPacket) ( unsigned char,
                                       unsigned char, 
                                       unsigned long, void *, 
                                       unsigned long, 
                                       unsigned long ) = NULL;
unsigned long (*SMDI_NextDataPacketRequest) ( unsigned char,
                                              unsigned char, 
                                              unsigned long, 
                                              void *, 
                                              unsigned long ) = NULL;
unsigned long (*SMDI_DeleteSample) ( unsigned char, 
                                     unsigned char, 
                                     unsigned long ) = NULL;
BOOL (*SMDI_TestUnitReady) ( unsigned char, unsigned char ) = NULL;
unsigned long (*SMDI_GetMessage) ( unsigned char, unsigned char ) = NULL;
unsigned long (*SMDI_SampleTransmission) ( SMDI_TransmissionInfo * ) = NULL;
unsigned long (*SMDI_InitSampleTransmission) ( SMDI_TransmissionInfo * ) = NULL;
unsigned long (*SMDI_InitSampleReception) ( SMDI_TransmissionInfo * ) = NULL;
unsigned long (*SMDI_SampleReception) ( SMDI_TransmissionInfo * ) = NULL;
unsigned long (*SMDI_GetFileSampleHeader) ( char *, SMDI_SampleHeader * ) = NULL;
unsigned long (*SMDI_InitFileSampleTransmission) ( SMDI_FileTransmissionInfo * ) = NULL;
unsigned long (*SMDI_FileSampleTransmission) ( SMDI_FileTransmissionInfo * ) = NULL;
unsigned long (*SMDI_InitFileSampleReception) ( SMDI_FileTransmissionInfo * ) = NULL;
unsigned long (*SMDI_FileSampleReception) ( SMDI_FileTransmissionInfo * ) = NULL;
unsigned long (*SMDI_SendFile) ( SMDI_FileTransfer * ) = NULL;
unsigned long (*SMDI_ReceiveFile) ( SMDI_FileTransfer * ) = NULL;
unsigned long (*SMDI_MasterIdentify) ( unsigned char, unsigned char );

#ifdef __cplusplus
}
#endif

#endif

