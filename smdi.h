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


#ifndef _SMDI_H
#define _SMDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef WORD
#define WORD unsigned short
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef BOOL
#define BOOL int
#endif
#ifndef CALLBACK
#define CALLBACK __stdcall
#endif
#ifndef FARPROC
#define FARPROC void*
#endif

// SMDI message ID's
#define	SMDIM_ERROR                     0x00000000
#define	SMDIM_MASTERIDENTIFY            0x00010000
#define	SMDIM_SLAVEIDENTIFY             0x00010001
#define	SMDIM_MESSAGEREJECT             0x00020000
#define	SMDIM_ACK                       0x01000000
#define	SMDIM_NAK                       0x01010000
#define	SMDIM_WAIT                      0x01020000
#define	SMDIM_SENDNEXTPACKET            0x01030000
#define	SMDIM_ENDOFPROCEDURE            0x01040000
#define	SMDIM_ABORTPROCEDURE            0x01050000
#define	SMDIM_DATAPACKET                0x01100000
#define	SMDIM_SAMPLEHEADERREQUEST       0x01200000
#define	SMDIM_SAMPLEHEADER              0x01210000
#define	SMDIM_SAMPLENAME                0x01230000
#define	SMDIM_DELETESAMPLE              0x01240000
#define	SMDIM_BEGINSAMPLETRANSFER       0x01220000
#define	SMDIM_TRANSFERACKNOWLEDGE       0x01220001
#define	SMDIM_TRANSMITMIDIMESSAGE       0x02000000

// SMDI Errors
#define	SMDIE_OUTOFRANGE                0x00200000
#define	SMDIE_NOSAMPLE                  0x00200002
#define	SMDIE_NOMEMORY                  0x00200004
#define SMDIE_UNSUPPSAMBITS             0x00200006

// Copy modes for SMDI_SendDataPacket
#define	CM_NORMAL                       0x00000000 // Does a 1:1 copy
#define	CM_SWAPDWORD                    0x00000001 // Longword swapping
#define	CM_SWAPWORD                     0x00000002 // Word swapping

// File types
#define FT_WAV                          0x00000001 // RIFF WAVE

// File errors
#define FE_OPENERROR                    0x00010001 // Couldn't open the file
#define	FE_UNKNOWNFORMAT                0x00010002 // Unsupported file format



typedef struct SCSI_DevInfo
{
  DWORD dwStructSize;                   // (00)
  BOOL bSMDI;                           // (04)
  BYTE DevType;                         // (08)
  BYTE Rsvd1;                           // (09)
  BYTE Rsvd2;                           // (10)
  BYTE Rsvd3;                           // (11)
  char cName[20];                       // (12)
  char cManufacturer[12];               // (32)
} SCSI_DevInfo;


typedef struct SMDI_SampleHeader
{
  DWORD dwStructSize;                   // (000)
  BOOL bDoesExist;                      // (004)
  BYTE BitsPerWord;                     // (008)
  BYTE NumberOfChannels;                // (009)
  BYTE LoopControl;                     // (010)
  BYTE NameLength;                      // (011)
  DWORD dwPeriod;                       // (012)
  DWORD dwLength;                       // (016)
  DWORD dwLoopStart;                    // (020)
  DWORD dwLoopEnd;                      // (024)
  WORD wPitch;                          // (028)
  WORD wPitchFraction;                  // (030)
  char cName[256];                      // (032)
  DWORD dwDataOffset;                   // (288)
} SMDI_SampleHeader;

typedef struct SMDI_TransmissionInfo    // For "manual" transmissions
{
  DWORD dwStructSize;                   // (00)
  SMDI_SampleHeader * lpSampleHeader;   // (04)
  DWORD dwTransmittedPackets;           // (08)
  DWORD dwPacketSize;                   // (12)
  DWORD dwSampleNumber;                 // (16)
  DWORD dwCopyMode;                     // (20)
  void * lpSampleData;                  // (24)
  BYTE SCSI_ID;                         // (28)
  BYTE HA_ID;                           // (29)
  BYTE Rsvd1;                           // (30)
  BYTE Rsvd2;                           // (31)
} SMDI_TransmissionInfo;

typedef struct SMDI_FileTransmissionInfo // For "automatic" file transmissions
{
  DWORD dwStructSize;
  void (*lpCallBackProcedure)();
  SMDI_TransmissionInfo * lpTransmissionInfo;
  DWORD dwFileType;
  FILE * hFile;
  char cFileName[MAX_PATH];
  DWORD * lpReturnValue;
  DWORD dwUserData;
} SMDI_FileTransmissionInfo;


typedef struct SMDI_FileTransfer
{
  DWORD dwStructSize;
  BYTE HA_ID;
  BYTE rsvd1;
  BYTE rsvd2;
  BYTE rsvd3;
  BYTE SCSI_ID;
  BYTE rsvd4;
  BYTE rsvd5;
  BYTE rsvd6;
  DWORD dwSampleNumber;
  char * lpFileName;
  DWORD dwFileType;                    // Only needed when receiving a file
  char * lpSampleName;                 // Only needed when sending a file
  FARPROC lpCallback;
  DWORD dwUserData;
  BOOL bAsync;
  DWORD * lpReturnValue;
} SMDI_FileTransfer;



#ifdef __cplusplus
}
#endif

#endif

