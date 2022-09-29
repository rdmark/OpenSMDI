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


#ifndef _SMDISTATIC_H
#define _SMDISTATIC_H

#include "smdi.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long SMDI_GetVersion ();
BOOL SMDI_Init ( void );
void SMDI_GetDeviceInfo ( unsigned char, unsigned char, SCSI_DevInfo * );
unsigned long SMDI_SampleHeaderRequest ( unsigned char,
                                         unsigned char, 
					 unsigned long, 
					 SMDI_SampleHeader * );
unsigned long SMDI_SendSampleHeader ( unsigned char,
                                      unsigned char, 
				      unsigned long, 
				      SMDI_SampleHeader *, 
				      unsigned long * );
unsigned long SMDI_SendBeginSampleTransfer ( unsigned char,
                                             unsigned char, 
					     unsigned long, 
					     void * );
unsigned long SMDI_SendDataPacket ( unsigned char,
                                    unsigned char, 
				    unsigned long, void *, 
				    unsigned long, 
				    unsigned long );
unsigned long SMDI_NextDataPacketRequest ( unsigned char,
                                           unsigned char, 
					   unsigned long, 
					   void *, 
					   unsigned long );
unsigned long SMDI_DeleteSample ( unsigned char, 
                                  unsigned char, 
                                  unsigned long );
BOOL SMDI_TestUnitReady ( unsigned char, unsigned char );
unsigned long SMDI_GetMessage ( unsigned char, unsigned char );
unsigned long SMDI_SampleTransmission ( SMDI_TransmissionInfo * );
unsigned long SMDI_InitSampleTransmission ( SMDI_TransmissionInfo * );
unsigned long SMDI_InitSampleReception ( SMDI_TransmissionInfo * );
unsigned long SMDI_SampleReception ( SMDI_TransmissionInfo * );
unsigned long SMDI_GetFileSampleHeader ( char *, SMDI_SampleHeader * );
unsigned long SMDI_InitFileSampleTransmission ( SMDI_FileTransmissionInfo * );
unsigned long SMDI_FileSampleTransmission ( SMDI_FileTransmissionInfo * );
unsigned long SMDI_InitFileSampleReception ( SMDI_FileTransmissionInfo * );
unsigned long SMDI_FileSampleReception ( SMDI_FileTransmissionInfo * );
unsigned long SMDI_SendFile ( SMDI_FileTransfer * );
unsigned long SMDI_ReceiveFile ( SMDI_FileTransfer * );
unsigned long SMDI_MasterIdentify ( unsigned char, unsigned char );

#ifdef __cplusplus
}
#endif

#endif

