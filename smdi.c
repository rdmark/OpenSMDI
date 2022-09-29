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


// The target Operating System. Only define one at a time.
#define _linux_
// #define _win32_

// The target processor. Only define one at a time.
#define _x86_

// Some compilers use "sleep", others "Sleep" - adjust it here if neccessary
#define __sleep(a) usleep(a)

// Macros for endian-adjusting
#define POFFSET(a, b) ((void *) (((unsigned long) (a)) + (b)))
#define ADJUSTL(a) ((a & 0xff000000) >> 24) | \
                   ((a & 0x00ff0000) >> 8)  | \
                   ((a & 0x0000ff00) << 8)  | \
                   ((a & 0x000000ff) << 24)
#define ADJUSTW(a) ((a & 0xff00) >> 8) | \
                   ((a & 0x00ff) << 8)
#define ADJUSTLM(a, b) { unsigned long dwTemp; \
                         memcpy ( &dwTemp, a, 4 ); \
                         dwTemp=ADJUSTL(dwTemp); \
                         memcpy ( b, &dwTemp, 4 ); }
#define ADJUSTWM(a, b) { unsigned short wTemp; \
                         memcpy ( &wTemp, a, 2 ); \
                         wTemp=ADJUSTW(wTemp); \
                         memcpy ( b, &wTemp, 2 ); }


#define PACKETSIZE 16384      // Standard data packet size
#define MAXFNLEN   260        // Maximum file name length, in bytes

#ifdef _linux_
#include <pthread.h>
#endif
#ifdef _win32_
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aspi.h"
#include "smdi.h"

// Version of SMDI.DLL
#define SMDIVERSION                     5 // 1 is 0.01, 2 0.02, 100 is 1.0,...

// Structures for the RIFF WAVE file format
typedef struct
{
  WORD wFormatTag;
  WORD wChannels;
  DWORD dwSamplesPerSec;
  DWORD dwAvgBytesPerSec;
  WORD wBlockAlign;
  WORD wBitsPerSample;
} WavFormatChunk;

typedef struct
{
  DWORD dwIdentifier;
  DWORD dwType;
  DWORD dwStart;
  DWORD dwEnd;
  DWORD dwFraction;
  DWORD dwPlayCount;
} WavSampleLoop;


typedef struct
{
  DWORD dwManufacturer;
  DWORD dwProduct;
  DWORD dwSamplePeriod;
  DWORD dwMIDIUnityNote;
  DWORD dwMIDIPitchFraction;
  DWORD dwSMPTEFormat;
  DWORD dwSMPTEOffset;
  DWORD cSampleLoops;
  DWORD cbSamplerData;
} WavSamplerChunk;



unsigned char smdicmd[256];


/* Returns the first 4 bytes of an address as a DWORD */
unsigned long GetStructSize ( void * srcStruct )
{
  unsigned long dwTemp;

  memcpy ( &dwTemp, srcStruct, 4 );
  return ( dwTemp );
}

/* Converts a 24 bit word into a DWORD and byte-swapps it */
void SMDI_Conv24B ( void * dwDest, void * cSrc )
{
  unsigned long dwDestTemp;
  unsigned char cSrcTemp[3];

  memcpy ( &cSrcTemp, cSrc, 3 );
  dwDestTemp = 0;
  dwDestTemp |= ((unsigned long) (cSrcTemp[0])) << 16;
  dwDestTemp |= ((unsigned long) (cSrcTemp[1])) << 8;
  dwDestTemp |= ((unsigned long) (cSrcTemp[2]));
  memcpy ( dwDest, &dwDestTemp, 4 );
}

/* Converts a DWORD into a 24 bit word and byte-swapps it */
void SMDI_ConvDWord ( void * cDest, void * dwSrc )
{
  unsigned long dwSrcTemp;
  unsigned char cDestTemp[3];

  memcpy ( &dwSrcTemp, dwSrc, 4 );
  cDestTemp[0] = (dwSrcTemp & 0x00ff0000) >> 16;
  cDestTemp[1] = (dwSrcTemp & 0x0000ff00) >> 8;
  cDestTemp[2] = (dwSrcTemp & 0x000000ff);
  memcpy ( cDest, &cDestTemp, 3 );
}


unsigned long SMDI_GetLastError ( void )
{
  unsigned long dummyL;

  ADJUSTLM ( &smdicmd[11], &dummyL );
  return (dummyL);
}


void SMDI_MakeMessageHeader ( 
			     unsigned char mheader[], 
			     unsigned long messageID, 
			     unsigned long AdditionalLength )
{
  memcpy ( mheader, "SMDI", 4 );
  ADJUSTLM ( &messageID, &mheader[4] );
  SMDI_ConvDWord ( &mheader[8], &AdditionalLength );
}

unsigned long SMDI_GetAdditionalLength ( unsigned char mheader[] )
{
  unsigned long dummyL;

  SMDI_Conv24B ( &dummyL, &mheader[8] );
  return dummyL;
}

unsigned short SMDI_GetMessageID ( unsigned char mheader[] )
{
  unsigned short dummyW;

  ADJUSTWM ( &mheader[4], &dummyW );
  return dummyW;
}

unsigned short SMDI_GetMessageSubID ( unsigned char mheader[] )
{
  unsigned short dummyW;

  ADJUSTWM ( &mheader[6], &dummyW );
  return dummyW;
}

unsigned long SMDI_GetWholeMessageID ( unsigned char mheader[] )
{
  return ( 
	  (((unsigned long) SMDI_GetMessageID(mheader)) << 16) | 
	   ((unsigned long) SMDI_GetMessageSubID(mheader)) );
}


__declspec(dllexport) unsigned long SMDI_GetMessage ( unsigned char ha_id, unsigned char id )
{
  ASPI_Receive ( ha_id, id, smdicmd, 256 );
  return (SMDI_GetWholeMessageID(smdicmd));
}


__declspec(dllexport) unsigned long 
SMDI_SendDataPacket ( unsigned char ha_id,
                      unsigned char id, 
		      unsigned long pn, 
		      void * data, 
		      unsigned long length, 
		      unsigned long copymode )
{
  void * datamessage;
  unsigned long i;

  datamessage = malloc ( 14 + length );
  if ( datamessage )
    {
      SMDI_MakeMessageHeader ( smdicmd, SMDIM_DATAPACKET, 3 + length );
      smdicmd[11] = (unsigned char) ((pn & 0x00ff0000) >> 16);
      smdicmd[12] = (unsigned char) ((pn & 0x0000ff00) >> 8);
      smdicmd[13] = (unsigned char)  (pn & 0x000000ff);
      memcpy ( datamessage, &smdicmd, 14 );

      if ( copymode==CM_NORMAL )
	{
	  memcpy ( (void *) (((unsigned long) datamessage)+14), data, length );
	}
      if ( copymode==CM_SWAPDWORD )
	{
	  for (i=0;i<length;i+=4)
	    {
	      ADJUSTLM ( POFFSET(data, i), POFFSET(datamessage, 14+i) );
	    }
	}
      if ( copymode==CM_SWAPWORD )
	{
	  for (i=0;i<length;i+=2)
	    {
	      ADJUSTWM ( POFFSET(data, i), POFFSET(datamessage, 14+i) );
	    }
	}
      ASPI_Send ( ha_id, id, datamessage, 14+length );
      ASPI_Receive ( ha_id, id, smdicmd, 256 );
      free ( datamessage );
      return (SMDI_GetWholeMessageID(smdicmd));
    } else
      {
	return (SMDIM_ERROR);
      }
}


__declspec(dllexport) unsigned long
SMDI_SampleName ( unsigned char ha_id,
                  unsigned char id, 
		  unsigned long sampleNum, 
		  char sampleName[] )
{
  SMDI_MakeMessageHeader ( smdicmd, 
			   SMDIM_SAMPLENAME, 
			   0x000004 + strlen(sampleName) );
  /* Sample Number */
  SMDI_ConvDWord ( &smdicmd[11], &sampleNum );
  /* Sample Name Length */
  smdicmd[14] = (unsigned char) strlen(sampleName);
  /* Sample Name */
  memcpy ( &smdicmd[15], sampleName, strlen(sampleName) );
  ASPI_Send ( ha_id, id, smdicmd, 15+strlen(sampleName) );
  ASPI_Receive ( ha_id, id, smdicmd, 256 );

  return (SMDI_GetWholeMessageID(smdicmd));
}


__declspec(dllexport) unsigned long
SMDI_SendBeginSampleTransfer ( unsigned char ha_id,
                               unsigned char id, 
			       unsigned long sampleNum, 
			       void * packetLength )
{
  SMDI_MakeMessageHeader ( smdicmd, 
			   SMDIM_BEGINSAMPLETRANSFER, 
			   0x000006 );
  /* Sample Number */
  SMDI_ConvDWord ( &smdicmd[11], &sampleNum );
  /* Data Packet Length */
  SMDI_ConvDWord ( &smdicmd[14], packetLength );

  ASPI_Send ( ha_id, id, smdicmd, 17 );
  ASPI_Receive ( ha_id, id, smdicmd, 256 );

  if (SMDI_GetWholeMessageID(smdicmd) == SMDIM_TRANSFERACKNOWLEDGE)
    {
      SMDI_Conv24B ( packetLength, &smdicmd[14] );
    }

  return (SMDI_GetWholeMessageID(smdicmd));
}


__declspec(dllexport) unsigned long
SMDI_SendSampleHeader ( unsigned char ha_id,
                        unsigned char id, 
			unsigned long sampleNum, 
			SMDI_SampleHeader * shA, 
			unsigned long * DataPacketLength )
{
  SMDI_SampleHeader sh;

  memcpy ( &sh, shA, GetStructSize ( shA ) );

  SMDI_MakeMessageHeader ( smdicmd, 
			   SMDIM_SAMPLEHEADER, 
			   0x00001a+ ((unsigned long) sh.NameLength) );
  /* Sample number */
  SMDI_ConvDWord ( &smdicmd[11], &sampleNum );
  /* Bits per word */
  smdicmd[14] = sh.BitsPerWord;
  /* Number of channels */
  smdicmd[15] = sh.NumberOfChannels;
  /* Sample Period (nanoseconds) */
  SMDI_ConvDWord ( &smdicmd[16], &sh.dwPeriod );
  /* Sample Length (words) */
  ADJUSTLM ( &sh.dwLength, &smdicmd[19] );
  /* Loop Start (word number) */
  ADJUSTLM ( &sh.dwLoopStart, &smdicmd[23] );
  /* Loop End (word number) */
  ADJUSTLM ( &sh.dwLoopEnd, &smdicmd[27] );
  /* Loop Control */
  smdicmd[31] = sh.LoopControl;
  /* Pitch Integer */
  ADJUSTWM ( &sh.wPitch, &smdicmd[32] );
  /* Pitch Fraction */
  ADJUSTWM ( &sh.wPitchFraction, &smdicmd[34] );
  /* Sample Name Length */
  smdicmd[36] = sh.NameLength;
  /* Sample Name */
  memcpy ( &smdicmd[37], &sh.cName, (unsigned long) sh.NameLength );

  ASPI_Send ( ha_id, id, smdicmd, 37+ ((unsigned long) sh.NameLength) );
  ASPI_Receive ( ha_id, id, smdicmd, 256 );

  if (SMDI_GetWholeMessageID(smdicmd)==SMDIM_TRANSFERACKNOWLEDGE)
    {
      SMDI_Conv24B ( DataPacketLength, &smdicmd[14] );
    }

  return (SMDI_GetWholeMessageID(smdicmd));
}


__declspec(dllexport) unsigned long
SMDI_NextDataPacketRequest ( unsigned char ha_id,
                             unsigned char id, 
			     unsigned long packetNumber, 
			     void * buffer, 
			     unsigned long maxlen, 
			     unsigned long copymode )
{
  unsigned long i;
  void * mybuffer;
  unsigned long reply;

  mybuffer = malloc ( maxlen+14 );
  SMDI_MakeMessageHeader ( smdicmd, SMDIM_SENDNEXTPACKET, 0x000003 );
  SMDI_ConvDWord ( &smdicmd[11], &packetNumber );
  ASPI_Send ( ha_id, id, smdicmd, 14 );
  ASPI_Receive ( ha_id, id, mybuffer, maxlen+14 );

  if ( copymode==CM_NORMAL )
    {
      memcpy ( buffer, (void *) (((unsigned long) mybuffer)+14), maxlen );
    }
  if ( copymode==CM_SWAPDWORD )
    {
      for (i=0;i<maxlen;i+=4)
	{
          ADJUSTLM ( POFFSET(mybuffer, 14+i), POFFSET(buffer, i) );
	}
    }
  if ( copymode==CM_SWAPWORD )
    {
      for (i=0;i<maxlen;i+=2)
	{
          ADJUSTWM ( POFFSET(mybuffer, 14+i), POFFSET(buffer, i) );
	}
    }

  reply = SMDI_GetWholeMessageID ( mybuffer );
  free ( mybuffer );
  return (reply);
}


__declspec(dllexport) unsigned long
SMDI_SampleHeaderRequest ( unsigned char ha_id,
                           unsigned char id, 
			   unsigned long sampleNum, 
			   SMDI_SampleHeader * shTemp )
{
  SMDI_SampleHeader sh;

  memcpy ( &sh, shTemp, GetStructSize ( shTemp ) );
  /* Send "SMDI Sample Header Request" */
  SMDI_MakeMessageHeader ( smdicmd, SMDIM_SAMPLEHEADERREQUEST, 0x000003 );
  SMDI_ConvDWord ( &smdicmd[11], &sampleNum );
  ASPI_Send ( ha_id, id, smdicmd, 14 );

  /* Read the response */
  ASPI_Receive ( ha_id, id, smdicmd, 256 );

  if ( SMDI_GetWholeMessageID(smdicmd) == SMDIM_SAMPLEHEADER )
    {
      sh.bDoesExist       = TRUE;
      sh.BitsPerWord      = smdicmd[14]; /* Bits Per Word */
      sh.NumberOfChannels = smdicmd[15]; /* Number Of Channels */

      /* Period */
      SMDI_Conv24B ( &sh.dwPeriod, &smdicmd[16] );
      sh.dwPeriod &= 0x00ffffff;

      /* Sample Length */
      ADJUSTLM ( &smdicmd[19], &sh.dwLength );

      /* Loop Start */
      ADJUSTLM ( &smdicmd[23], &sh.dwLoopStart );

      /* Loop End */
      ADJUSTLM ( &smdicmd[27], &sh.dwLoopEnd );

      /* Loop Control */
      sh.LoopControl = smdicmd[31];

      /* Name length */
      sh.NameLength = smdicmd[36];

      /* Name */
      memset ( &sh.cName, 0, 256 );
      memcpy ( sh.cName, 
	       &smdicmd[37], 
	       (unsigned long) sh.NameLength );	/* Name */

    } else
      {
	sh.bDoesExist = FALSE;
      }
  memcpy ( shTemp, &sh, GetStructSize ( &sh ) );

  if ( sh.bDoesExist == TRUE )
    {

      return (SMDI_GetWholeMessageID(smdicmd));
    } else
      {
	return (SMDI_GetLastError());
      }
}


__declspec(dllexport) unsigned long
SMDI_DeleteSample ( unsigned char ha_id,
                    unsigned char id, 
		    unsigned long sampleNum )
{
  SMDI_MakeMessageHeader ( smdicmd, SMDIM_DELETESAMPLE, 0x000003 );
  SMDI_ConvDWord ( &smdicmd[11], &sampleNum );
  ASPI_Send ( ha_id, id, smdicmd, 14 );
  ASPI_Receive ( ha_id, id, smdicmd, 32 );
  if ( SMDI_GetWholeMessageID(smdicmd) == SMDIM_MESSAGEREJECT )
    {
      return ( SMDI_GetLastError() );
    } else
      {
	return ( SMDI_GetWholeMessageID(smdicmd) );
      }
}


__declspec(dllexport) unsigned long
SMDI_MasterIdentify ( unsigned char ha_id, unsigned char id )
{
  /* Send "SMDI Master Identify" */
  SMDI_MakeMessageHeader ( smdicmd, SMDIM_MASTERIDENTIFY, 0x00000000 );

  ASPI_Send ( ha_id, id, smdicmd, 11 );

  /* Read the response */
  ASPI_Receive ( ha_id, id, smdicmd, 256 );

  return ( SMDI_GetWholeMessageID(smdicmd) );
}


__declspec(dllexport) unsigned long SMDI_GetVersion ()
{
  return ( SMDIVERSION );
}


__declspec(dllexport) unsigned char SMDI_Init ()
{
  return (ASPI_Check());
}


__declspec(dllexport) BOOL
SMDI_TestUnitReady ( unsigned char ha_id, unsigned char id )
{
  return (ASPI_TestUnitReady(ha_id, id));
}


__declspec(dllexport) unsigned long
SMDI_InitSampleReception ( SMDI_TransmissionInfo * tiATemp )
{
  SMDI_TransmissionInfo tiTemp;
  unsigned long messRet;

  memcpy ( &tiTemp, tiATemp, GetStructSize ( tiATemp ) );
  tiTemp.dwTransmittedPackets = 0;

  messRet = SMDI_SampleHeaderRequest ( tiTemp.HA_ID,
                                       tiTemp.SCSI_ID, 
				       tiTemp.dwSampleNumber, 
				       tiTemp.lpSampleHeader );
  if ( messRet == SMDIM_SAMPLEHEADER )
    {
      tiTemp.dwPacketSize = PACKETSIZE;
      messRet = SMDI_SendBeginSampleTransfer ( tiTemp.HA_ID,
                                               tiTemp.SCSI_ID, 
					       tiTemp.dwSampleNumber, 
					       &tiTemp.dwPacketSize );
    }

  if (messRet==SMDIM_MESSAGEREJECT)
    {
      return (SMDI_GetLastError());
    }

  memcpy ( tiATemp, &tiTemp, GetStructSize ( &tiTemp ) );
  return (messRet);
}


__declspec(dllexport) unsigned long
SMDI_SampleReception ( SMDI_TransmissionInfo * lpTransmissionInfo )
{
  SMDI_TransmissionInfo TransmissionInfo;
  SMDI_SampleHeader SampleHeader;
  unsigned long messRet;
  unsigned long TransmittedBytes;
  unsigned long SamLength;

  memcpy ( &TransmissionInfo,
	   lpTransmissionInfo, 
	   GetStructSize ( lpTransmissionInfo ) );
  memcpy ( &SampleHeader, 
	   TransmissionInfo.lpSampleHeader, 
	   GetStructSize ( TransmissionInfo.lpSampleHeader ) );

  TransmittedBytes = 
    (TransmissionInfo.dwPacketSize * TransmissionInfo.dwTransmittedPackets);

  SamLength = (
     SampleHeader.dwLength * 
     (unsigned long) SampleHeader.NumberOfChannels * 
     (unsigned long) SampleHeader.BitsPerWord ) / 8;

  messRet = 
    SMDI_NextDataPacketRequest ( TransmissionInfo.HA_ID,
                                 TransmissionInfo.SCSI_ID, 
				 TransmissionInfo.dwTransmittedPackets, 
				 TransmissionInfo.lpSampleData, 
				 TransmissionInfo.dwPacketSize, 
				 TransmissionInfo.dwCopyMode );

  if ( (TransmittedBytes+TransmissionInfo.dwPacketSize) >= SamLength)
    messRet = SMDIM_ENDOFPROCEDURE;

  TransmissionInfo.dwTransmittedPackets++;

  memcpy ( lpTransmissionInfo,
	   &TransmissionInfo, 
	   GetStructSize ( &TransmissionInfo ) );

  return ( messRet );
}


__declspec(dllexport) unsigned long
SMDI_InitSampleTransmission ( SMDI_TransmissionInfo * lpTransmissionInfo )
{
  BOOL unitready;
  SMDI_TransmissionInfo TransmissionInfo;
  unsigned long messRet;

  memcpy ( &TransmissionInfo, 
	   lpTransmissionInfo, 
	   GetStructSize ( lpTransmissionInfo ) );

  TransmissionInfo.dwTransmittedPackets = 0;
  TransmissionInfo.dwPacketSize = PACKETSIZE;

  messRet = SMDI_SendSampleHeader ( TransmissionInfo.HA_ID,
                                    TransmissionInfo.SCSI_ID, 
				    TransmissionInfo.dwSampleNumber, 
				    TransmissionInfo.lpSampleHeader, 
				    &TransmissionInfo.dwPacketSize );
  if (messRet == SMDIM_TRANSFERACKNOWLEDGE)
    {
      messRet = 
		SMDI_SendBeginSampleTransfer ( TransmissionInfo.HA_ID,
                                       TransmissionInfo.SCSI_ID, 
									   TransmissionInfo.dwSampleNumber, 
									   &TransmissionInfo.dwPacketSize );
      if (messRet == SMDIM_WAIT)
		{
		  unitready = FALSE;
		  while (unitready==FALSE)
			{
#ifdef _linux_
			  __sleep(100);
			  printf("Waiting for unit to become ready\n");
#endif
#ifdef _win32_
			  __sleep(500);
#endif
			  unitready = ASPI_TestUnitReady ( TransmissionInfo.HA_ID, 
                                               TransmissionInfo.SCSI_ID );
			}
		  messRet = SMDI_GetMessage ( TransmissionInfo.HA_ID, 
                                      TransmissionInfo.SCSI_ID );
		}
    }
  
  if (messRet==SMDIM_MESSAGEREJECT)
    return (SMDI_GetLastError());

  memcpy ( lpTransmissionInfo, 
	   &TransmissionInfo, 
	   GetStructSize ( &TransmissionInfo ) );
  return (messRet);
}


__declspec(dllexport) unsigned long
SMDI_SampleTransmission ( SMDI_TransmissionInfo * lpTransmissionInfo )
{
  SMDI_SampleHeader SampleHeader;
  SMDI_TransmissionInfo TransmissionInfo;
  BOOL ur;
  unsigned long messRet;
  unsigned long TransmittedBytes;
  unsigned long SamLength;
  unsigned long timeout;

  memcpy ( &TransmissionInfo, 
	   lpTransmissionInfo, 
	   GetStructSize ( lpTransmissionInfo ) );
  memcpy ( &SampleHeader, 
	   TransmissionInfo.lpSampleHeader, 
	   GetStructSize ( TransmissionInfo.lpSampleHeader ) );

  TransmittedBytes = 
    (TransmissionInfo.dwPacketSize * TransmissionInfo.dwTransmittedPackets);
  SamLength = 
    (SampleHeader.dwLength * 
     (unsigned long) SampleHeader.NumberOfChannels * 
     (unsigned long) SampleHeader.BitsPerWord) / 8;

  if ( (TransmittedBytes + TransmissionInfo.dwPacketSize) > SamLength )
      TransmissionInfo.dwPacketSize = SamLength - TransmittedBytes;

  messRet = 
    SMDI_SendDataPacket ( TransmissionInfo.HA_ID,
                          TransmissionInfo.SCSI_ID, 
						  TransmissionInfo.dwTransmittedPackets, 
						  TransmissionInfo.lpSampleData, 
						  TransmissionInfo.dwPacketSize, 
						  TransmissionInfo.dwCopyMode );
  if (messRet == SMDIM_WAIT)
	{
	  ur = FALSE;
	  while (ur==FALSE)
		{
#ifdef _linux_
		  __sleep(100);

#endif
#ifdef _win32_
		  __sleep(500);
#endif
		  ur = ASPI_TestUnitReady ( TransmissionInfo.HA_ID, 
									TransmissionInfo.SCSI_ID );
		}
	  messRet = SMDI_GetMessage ( TransmissionInfo.HA_ID, 
								  TransmissionInfo.SCSI_ID );
	}

#if 0
  if (messRet==SMDIM_MESSAGEREJECT)
    return (SMDI_GetLastError());

  if (messRet==SMDIM_WAIT)
    {
      ur = FALSE;
	  timeout=0;
      while (ur==FALSE && timeout<2000) 
		{ 
		  printf("Got a \"wait\" from the sampler. Waiting 100 ms.\n");
		  fflush(stdout);
		  __sleep(100000); 
		  timeout+=100;
		  /*
		  ur=ASPI_TestUnitReady( TransmissionInfo.HA_ID,
								 TransmissionInfo.SCSI_ID) ;
		  */
		  ASPI_Receive(TransmissionInfo.HA_ID,
					   TransmissionInfo.SCSI_ID,
					   smdicmd, 256);
		  if (messRet!=SMDIM_SENDNEXTPACKET)
			{
			  printf("Waited and got %x. What a shame.\n", messRet);
			}
		}
    }
#endif


  TransmissionInfo.dwTransmittedPackets++;
  memcpy ( lpTransmissionInfo, 
	   &TransmissionInfo, 
	   GetStructSize ( &TransmissionInfo ) );

  return messRet;
}


__declspec(dllexport) void 
SMDI_GetDeviceInfo ( unsigned char ha_id, unsigned char id, SCSI_DevInfo * lpDevInfo )
{
  SCSI_DevInfo DevInfo;
  char inquire[96];

  memset ( inquire, 0, 96 );
  DevInfo.dwStructSize = GetStructSize ( lpDevInfo );

  ASPI_InquireDevice ( inquire, ha_id, id );
  DevInfo.DevType = inquire[0] & 0x1f;
  DevInfo.bSMDI = FALSE;

  memset ( &DevInfo.cName, 0, 20 );
  memset ( &DevInfo.cManufacturer, 0, 12 );
  memcpy ( DevInfo.cName, &inquire[16], 16 );
  memcpy ( DevInfo.cManufacturer, &inquire[8], 8 );

  if ( DevInfo.DevType==3 )
      if (SMDI_MasterIdentify(ha_id, id)==SMDIM_SLAVEIDENTIFY)
	  DevInfo.bSMDI = TRUE;

  memcpy ( lpDevInfo, &DevInfo, GetStructSize ( &DevInfo ) );
}


__declspec(dllexport) unsigned long
SMDI_GetFileSampleHeader ( char cFileName[MAXFNLEN], 
			   SMDI_SampleHeader * lpSampleHeader )
{
  FILE * hFHandle;
  WavFormatChunk wfcTemp;
  WavSamplerChunk wscTemp;
  WavSampleLoop wslTemp;
  char curChunkName[5];
  SMDI_SampleHeader shMyTemp;
  unsigned long curChunkLen;
  unsigned long curChunkPos;
  char curChunkName2[5];
  unsigned long dwProcessedBytes;
  unsigned long dwSize;

  memcpy ( &shMyTemp, lpSampleHeader, GetStructSize ( lpSampleHeader ) );
  curChunkName[4] = 0;
  curChunkName2[4] = 0;
  shMyTemp.LoopControl = 127;
  shMyTemp.wPitch = 60;
  shMyTemp.wPitchFraction = 0;
  shMyTemp.dwLoopStart = 0;
  shMyTemp.dwLoopEnd = 0;
  hFHandle = fopen ( cFileName, "rb" );
  if (hFHandle != NULL)
    {
      fread ( &curChunkName, 1, 4, hFHandle );
      fread ( &dwSize, 1, 4, hFHandle );
      fread ( &curChunkName2, 1, 4, hFHandle );
      if ( (strcmp(curChunkName,"RIFF")==0) && 
	   (strcmp(curChunkName2,"WAVE")==0) )
	{
	  dwProcessedBytes = 12;
	  /* we have a wav file ! */
	  while ( dwProcessedBytes < dwSize ) /* read chunks */
	    {
	      fread ( &curChunkName, 1, 4, hFHandle );
	      fread ( &curChunkLen, 1, 4, hFHandle );
	      fgetpos ( hFHandle, (fpos_t *) &curChunkPos );
	      dwProcessedBytes += curChunkLen+4;
	      if ( memcmp(curChunkName, "fmt ", 4)==0 )
		{
		  fread ( &wfcTemp, 1, sizeof(wfcTemp), hFHandle );

		  shMyTemp.bDoesExist = TRUE;
		  shMyTemp.BitsPerWord = wfcTemp.wBitsPerSample;
		  shMyTemp.NumberOfChannels = wfcTemp.wChannels;
		  shMyTemp.dwPeriod = 1000000000 / wfcTemp.dwSamplesPerSec;
		}
	      if ( memcmp(curChunkName, "smpl", 4)==0 )
		{
		  fread ( &wscTemp, 1, sizeof(wscTemp), hFHandle );

		  shMyTemp.wPitch = wscTemp.dwMIDIUnityNote;
		  shMyTemp.wPitchFraction = wscTemp.dwMIDIPitchFraction;

		  fread ( &wslTemp, 1, sizeof(wslTemp), hFHandle );

		  shMyTemp.dwLoopStart = 
		    (wslTemp.dwStart * 8) / 
		    (shMyTemp.NumberOfChannels * shMyTemp.BitsPerWord);
		  shMyTemp.dwLoopEnd = 
		    (wslTemp.dwEnd * 8) / 
		    (shMyTemp.NumberOfChannels * shMyTemp.BitsPerWord);

		  shMyTemp.LoopControl = 0;
		}
	      if ( memcmp(curChunkName, "data", 4)==0 )
		{
		  fgetpos ( hFHandle, (fpos_t *) &shMyTemp.dwDataOffset );
		  shMyTemp.dwLength = curChunkLen;
		}
	      fseek ( hFHandle, curChunkPos+curChunkLen, SEEK_SET );
	    }
	  fclose ( hFHandle );
	  shMyTemp.dwLength = 
	    (shMyTemp.dwLength * 8) / 
	    (shMyTemp.BitsPerWord * shMyTemp.NumberOfChannels);

	  if ( shMyTemp.dwLoopEnd == 0 )
	    shMyTemp.dwLoopEnd = shMyTemp.dwLength - 1;
	  memcpy ( lpSampleHeader, &shMyTemp, GetStructSize ( &shMyTemp ) );

	  printf("Sample size: %d\n", dwSize);
	  return ( FT_WAV );
	}
      /* <-- here comes the next format */

      /* no format */
      fclose ( hFHandle );
      return ( FE_UNKNOWNFORMAT );
    } else
      {
	return ( FE_OPENERROR );
      }
}


__declspec(dllexport) unsigned long
SMDI_FileSampleTransmission ( 
			     SMDI_FileTransmissionInfo * 
			     lpFileTransmissionInfo )
{
  SMDI_FileTransmissionInfo ftiTemp;
  SMDI_TransmissionInfo tiTemp;
  SMDI_SampleHeader shTemp;
  unsigned long dwTemp;

  memcpy ( &ftiTemp, 
	   lpFileTransmissionInfo, 
	   GetStructSize ( lpFileTransmissionInfo ) );
  memcpy ( &tiTemp, 
	   ftiTemp.lpTransmissionInfo, 
	   GetStructSize ( ftiTemp.lpTransmissionInfo ) );
  memcpy ( &shTemp, 
	   tiTemp.lpSampleHeader, 
	   GetStructSize ( tiTemp.lpSampleHeader ) );

  fread ( tiTemp.lpSampleData, 1, tiTemp.dwPacketSize, ftiTemp.hFile );
  dwTemp = SMDI_SampleTransmission ( &tiTemp );
  if (dwTemp == SMDIM_ENDOFPROCEDURE)
	{
      free ( tiTemp.lpSampleData );
      fclose ( ftiTemp.hFile );
	  return dwTemp;
	}
  else if ( dwTemp != SMDIM_SENDNEXTPACKET )
    {
      free ( tiTemp.lpSampleData );
      fclose ( ftiTemp.hFile );
    }

  memcpy ( tiTemp.lpSampleHeader, &shTemp, GetStructSize ( &shTemp ) );
  memcpy ( ftiTemp.lpTransmissionInfo, &tiTemp, GetStructSize ( &tiTemp ) );
  memcpy ( lpFileTransmissionInfo, &ftiTemp, GetStructSize ( &ftiTemp ) );
  return ( dwTemp );
}


__declspec(dllexport) unsigned long
SMDI_InitFileSampleTransmission ( 
				 SMDI_FileTransmissionInfo * 
				 lpFileTransmissionInfo )
{
  SMDI_FileTransmissionInfo ftiTemp;
  SMDI_TransmissionInfo tiTemp;
  SMDI_SampleHeader shTemp;
  unsigned long dwTemp;

  memcpy ( &ftiTemp, 
	   lpFileTransmissionInfo, 
	   GetStructSize ( lpFileTransmissionInfo ) );
  memcpy ( &tiTemp, 
	   ftiTemp.lpTransmissionInfo, 
	   GetStructSize ( ftiTemp.lpTransmissionInfo ) );
  dwTemp = SMDI_GetFileSampleHeader ( ftiTemp.cFileName, 
				      tiTemp.lpSampleHeader );

  
  memcpy ( &shTemp, 
	   tiTemp.lpSampleHeader, 
	   GetStructSize ( tiTemp.lpSampleHeader ) );

  shTemp.wPitch = 0;
  shTemp.wPitchFraction = 0;
  if ( dwTemp < 0x00010001 )
    {
      switch ( dwTemp )	/* determine the byte orders in sample data */
	{
	case FT_WAV:
	  /* 16bit stereo => dword */
	  if ( (shTemp.BitsPerWord == 16) && (shTemp.NumberOfChannels == 2) )
	    tiTemp.dwCopyMode = CM_SWAPDWORD;
	  /* 8bit stereo => word */
	  if ( (shTemp.BitsPerWord == 8 ) && (shTemp.NumberOfChannels == 2) )
	    tiTemp.dwCopyMode = CM_SWAPWORD;
	  /* 16bit mono => word */
	  if ( (shTemp.BitsPerWord == 16) && (shTemp.NumberOfChannels == 1) )
	    tiTemp.dwCopyMode = CM_SWAPWORD;
	  /* 8bit mono => normal */
	  if ( (shTemp.BitsPerWord == 8 ) && (shTemp.NumberOfChannels == 1) )
	    tiTemp.dwCopyMode = CM_NORMAL;
	  break;
	}
      dwTemp = SMDI_InitSampleTransmission ( &tiTemp );

      if ( dwTemp == SMDIM_SENDNEXTPACKET )
	{
	  ftiTemp.hFile = fopen ( ftiTemp.cFileName, "rb" );
	  fseek ( ftiTemp.hFile, shTemp.dwDataOffset, SEEK_SET );
	  tiTemp.lpSampleData = malloc ( tiTemp.dwPacketSize );
	}
      memcpy ( tiTemp.lpSampleHeader, 
	       &shTemp, 
	       GetStructSize ( &shTemp ) );
      memcpy ( ftiTemp.lpTransmissionInfo, 
	       &tiTemp, 
	       GetStructSize ( &tiTemp ) );
      memcpy ( lpFileTransmissionInfo, 
	       &ftiTemp, 
	       GetStructSize ( &ftiTemp ) );
    }

  return (dwTemp);
}


__declspec(dllexport) unsigned long
SMDI_FileSampleReception ( 
			  SMDI_FileTransmissionInfo * 
			  lpFileTransmissionInfo )
{
  SMDI_FileTransmissionInfo ftiTemp;
  SMDI_TransmissionInfo tiTemp;
  SMDI_SampleHeader shTemp;
  unsigned long dwTemp;
  unsigned long dwBytesToWrite;

  memcpy ( &ftiTemp, 
	   lpFileTransmissionInfo, 
	   GetStructSize ( lpFileTransmissionInfo ) );
  memcpy ( &tiTemp, 
	   ftiTemp.lpTransmissionInfo, 
	   GetStructSize ( ftiTemp.lpTransmissionInfo ) );
  memcpy ( &shTemp, 
	   tiTemp.lpSampleHeader, 
	   GetStructSize ( tiTemp.lpSampleHeader ) );
  dwTemp = SMDI_SampleReception ( &tiTemp );

  dwBytesToWrite = tiTemp.dwPacketSize;

  if ( dwTemp != SMDIM_DATAPACKET )
    {
      dwBytesToWrite =
	(shTemp.dwLength * 
	 ((unsigned long) shTemp.NumberOfChannels) * 
	 (((unsigned long) shTemp.BitsPerWord)/8)) -
	(tiTemp.dwPacketSize * 
	 (tiTemp.dwTransmittedPackets-1));
    }
  fwrite ( tiTemp.lpSampleData, 1, dwBytesToWrite, ftiTemp.hFile );
  if ( dwTemp == SMDIM_ENDOFPROCEDURE )
    {
      fclose ( ftiTemp.hFile );
      free ( tiTemp.lpSampleData );
    }

  memcpy ( ftiTemp.lpTransmissionInfo, 
	   &tiTemp, 
	   GetStructSize ( &tiTemp ) );
  return ( dwTemp );
}


__declspec(dllexport) unsigned long
SMDI_InitFileSampleReception ( 
			      SMDI_FileTransmissionInfo * 
			      lpFileTransmissionInfo )
{
  SMDI_FileTransmissionInfo ftiTemp;
  SMDI_TransmissionInfo tiTemp;
  SMDI_SampleHeader shTemp;
  unsigned long dwTemp;
  unsigned short wTemp;

  memcpy ( &ftiTemp, 
	   lpFileTransmissionInfo, 
	   GetStructSize ( lpFileTransmissionInfo ) );
  memcpy ( &tiTemp, 
	   ftiTemp.lpTransmissionInfo, 
	   GetStructSize ( ftiTemp.lpTransmissionInfo ) );
  memcpy ( &shTemp, 
	   tiTemp.lpSampleHeader, 
	   GetStructSize ( tiTemp.lpSampleHeader ) );

  dwTemp = SMDI_SampleHeaderRequest ( tiTemp.HA_ID,
                                      tiTemp.SCSI_ID, 
				      tiTemp.dwSampleNumber, 
				      &shTemp );
  if ( dwTemp != SMDIM_SAMPLEHEADER ) return ( dwTemp );

  tiTemp.dwPacketSize = PACKETSIZE;

  ftiTemp.hFile = fopen ( ftiTemp.cFileName, "w+b" );
  if ( ftiTemp.hFile == NULL ) { return ( FE_OPENERROR ); }
  switch ( ftiTemp.dwFileType )
    {
    case FT_WAV:
      /* Write the global header */
      fwrite ( "RIFF", 1, 4, ftiTemp.hFile );
      dwTemp = 4;
      dwTemp += 8+16;  /* The fmt chunk */
      dwTemp += 8+ 
	(shTemp.dwLength * ((unsigned long) shTemp.NumberOfChannels) * 
	 (((unsigned long) shTemp.BitsPerWord)/8) ); /* The data chunk */
      if ( shTemp.LoopControl != 127 ) dwTemp += 44+24; /* the smpl chunk */
      fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
      fwrite ( "WAVE", 1, 4, ftiTemp.hFile );

      /* Write the fmt chunk */
      /* id, size */
      fwrite ( "fmt ", 1, 4, ftiTemp.hFile );
      dwTemp = 16;
      fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
      /* wav format (always 1 = not compressed) */
      wTemp = 1;
      fwrite ( &wTemp, 1, 2, ftiTemp.hFile );
      /* number of channels */
      wTemp = (unsigned short) shTemp.NumberOfChannels;
      fwrite ( &wTemp, 1, 2, ftiTemp.hFile );
      /* samples per second */
      dwTemp = 1000000000 / shTemp.dwPeriod;
      fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
      /* average bytes per second */
      dwTemp = 
	(1000000000 / shTemp.dwPeriod) * 
	((unsigned long) shTemp.NumberOfChannels) * 
	(((unsigned long) shTemp.BitsPerWord)/8);
      fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
      /* bytes per sample */
      wTemp = 
	((unsigned short) shTemp.NumberOfChannels) * 
	(((unsigned short) shTemp.BitsPerWord)/8);
      fwrite ( &wTemp, 1, 2, ftiTemp.hFile );
      /* bits per sample */
      wTemp = (unsigned short) shTemp.BitsPerWord;
      fwrite ( &wTemp, 1, 2, ftiTemp.hFile );

      /* write the smpl chunk if there's a loop */
      if ( shTemp.LoopControl != 127 )
	{
	  /* id+size */
	  fwrite ( "smpl", 1, 4, ftiTemp.hFile );
	  dwTemp = 60;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* manufacturer */
	  dwTemp = 0;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* product */
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* sample period */
	  dwTemp = shTemp.dwPeriod;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* MIDI unity note */
	  dwTemp = 60;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* MIDI pitch fraction */
	  dwTemp = 0;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* SMPTE format */
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* SMPTE offset */
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* sample loops */
	  dwTemp = 1;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* sampler data */
	  dwTemp = 0;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* THE LOOP: */
	  /* identifier */
	  dwTemp = 0;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* type */
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* start */
	  dwTemp = shTemp.dwLoopStart;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* end */
	  dwTemp = shTemp.dwLoopEnd;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* fraction */
	  dwTemp = 0;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	  /* play count */
	  dwTemp = 0;
	  fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );
	}

      /* Now we write the data chunk */
      fwrite ( "data", 1, 4, ftiTemp.hFile );
      dwTemp = 
	shTemp.dwLength * 
	((unsigned long) shTemp.NumberOfChannels) * 
	(((unsigned long) shTemp.BitsPerWord)/8);
      fwrite ( &dwTemp, 1, 4, ftiTemp.hFile );

      dwTemp = SMDI_InitSampleReception ( &tiTemp );
      if ( dwTemp != SMDIM_TRANSFERACKNOWLEDGE )
	{
	  fclose ( ftiTemp.hFile );
	  remove ( ftiTemp.cFileName );
	  return ( dwTemp );
	}

      /* Determine the CopyMode */
      /* 16bit stereo => dword */
      if ( (shTemp.BitsPerWord == 16) && (shTemp.NumberOfChannels == 2) )
	tiTemp.dwCopyMode = CM_SWAPDWORD;
      /* 8bit stereo => word */
      if ( (shTemp.BitsPerWord == 8 ) && (shTemp.NumberOfChannels == 2) )
	tiTemp.dwCopyMode = CM_SWAPWORD;
      /* 16bit mono => word */
      if ( (shTemp.BitsPerWord == 16) && (shTemp.NumberOfChannels == 1) )
	tiTemp.dwCopyMode = CM_SWAPWORD;
      /* 8bit mono => normal */
      if ( (shTemp.BitsPerWord == 8 ) && (shTemp.NumberOfChannels == 1) )
	tiTemp.dwCopyMode = CM_NORMAL;

      tiTemp.lpSampleData = malloc ( tiTemp.dwPacketSize );
      memcpy ( tiTemp.lpSampleHeader, 
	       &shTemp, 
	       GetStructSize ( &shTemp ) );
      memcpy ( ftiTemp.lpTransmissionInfo, 
	       &tiTemp, 
	       GetStructSize ( &tiTemp ) );
      memcpy ( lpFileTransmissionInfo, 
	       &ftiTemp, 
	       GetStructSize ( &ftiTemp ) );
      return ( dwTemp );
    default:
      return ( FE_UNKNOWNFORMAT );
    }
}


/* This is the main-send function which can optionally be threaded */
unsigned long SMDI_SendFileMain ( LPVOID lpStart  )
{
  SMDI_FileTransmissionInfo ftiTemp;
  SMDI_TransmissionInfo tiTemp;
  SMDI_SampleHeader shTemp;
  unsigned long dwTemp;

  memcpy (
    &ftiTemp,
    lpStart,
    sizeof(ftiTemp) );
  memcpy (
    &tiTemp,
    (void *) ( (unsigned long) lpStart + sizeof(ftiTemp) ),
    sizeof(tiTemp) );
  memcpy (
    &shTemp,
    (void *) ( (unsigned long) lpStart + sizeof(ftiTemp) + sizeof(tiTemp) ),
    sizeof(shTemp) );

  ftiTemp.lpTransmissionInfo = &tiTemp;
  tiTemp.lpSampleHeader = &shTemp;

  free ( lpStart );

  dwTemp = SMDI_InitFileSampleTransmission ( &ftiTemp );
  if ( dwTemp == SMDIM_SENDNEXTPACKET )
    {
      dwTemp = SMDIM_SENDNEXTPACKET;
      while ( dwTemp == SMDIM_SENDNEXTPACKET)
		{
		  dwTemp = SMDI_FileSampleTransmission ( &ftiTemp );
		  if (ftiTemp.lpCallBackProcedure != NULL)
			(*ftiTemp.lpCallBackProcedure) ( &ftiTemp, ftiTemp.dwUserData );
		}
    }
  if ( ftiTemp.lpReturnValue != NULL )
    *(ftiTemp.lpReturnValue) = dwTemp;
  return ( dwTemp );
}


__declspec(dllexport) unsigned long
SMDI_SendFile ( SMDI_FileTransfer * lpFileTransfer )
{
  SMDI_FileTransmissionInfo * ftiTemp;
  SMDI_TransmissionInfo     * tiTemp;
  SMDI_SampleHeader         * shTemp;
  void * lpTemp;
  SMDI_FileTransfer FileTransfer;
  unsigned long dwThreadID;

  /* Getting mem for the structures */
  ftiTemp = malloc ( sizeof(*ftiTemp) + sizeof(*tiTemp) + sizeof(*shTemp) );
  lpTemp = ftiTemp;
  tiTemp  = (void *) ((DWORD)ftiTemp + (DWORD)sizeof(*ftiTemp));
  shTemp  = (void *) ((DWORD)ftiTemp + (DWORD)sizeof(*ftiTemp) + (DWORD)sizeof(*tiTemp));

  /* Setting default values for the supplied structure */
  FileTransfer.dwStructSize = sizeof(FileTransfer);
  FileTransfer.HA_ID = 0;
  FileTransfer.SCSI_ID = 0;
  FileTransfer.dwSampleNumber = 0;
  FileTransfer.lpFileName = NULL;
  FileTransfer.lpSampleName = NULL;
  FileTransfer.bAsync = FALSE;
  FileTransfer.lpCallback = NULL;
  FileTransfer.dwUserData = 0;
  FileTransfer.lpReturnValue = NULL;

  /* Getting the supplied structure */
  memcpy ( &FileTransfer, 
    lpFileTransfer, 
    ((*lpFileTransfer).dwStructSize > sizeof(FileTransfer)) ? sizeof(FileTransfer) : (*lpFileTransfer).dwStructSize );

  /* Initialize the structures */
  (*ftiTemp).dwStructSize = sizeof ( *ftiTemp );
  (*tiTemp).dwStructSize = sizeof ( *tiTemp );
  (*shTemp).dwStructSize = sizeof ( *shTemp );

  (*tiTemp).HA_ID = FileTransfer.HA_ID;
  (*tiTemp).SCSI_ID = FileTransfer.SCSI_ID;
  (*tiTemp).dwSampleNumber = FileTransfer.dwSampleNumber;
  strcpy ( ((*ftiTemp).cFileName), FileTransfer.lpFileName );
  strcpy ( ((*shTemp).cName), FileTransfer.lpSampleName );
  (*shTemp).NameLength = (BYTE) strlen((*shTemp).cName);
  (*ftiTemp).lpCallBackProcedure = FileTransfer.lpCallback;
  (*ftiTemp).lpReturnValue = FileTransfer.lpReturnValue;
  (*ftiTemp).dwUserData = FileTransfer.dwUserData;

  /* Set references to each other */
  (*ftiTemp).lpTransmissionInfo = tiTemp;
  (*tiTemp).lpSampleHeader = shTemp;

  if (FileTransfer.lpReturnValue != NULL) 
	*(FileTransfer.lpReturnValue) = -1;

  fflush(stdout);

#ifdef _win32_
  if (FileTransfer.bAsync==TRUE)
    {
      CreateThread ( NULL, 
			     0, 
			     (unsigned long (__stdcall *)(void *)) SMDI_SendFileMain, 
			     lpTemp, 
			     0, 
			     &dwThreadID );
      return ( -1 );
    } else
      return ( SMDI_SendFileMain ( lpTemp ) );
#endif
#ifdef _linux_
  fflush(stdout);
  if (FileTransfer.bAsync==TRUE)
    {
      pthread_create ( &dwThreadID, 
		       NULL, 
		       (void*) SMDI_SendFileMain, 
		       lpTemp );
      return ( -1 );
    } else
      return ( SMDI_SendFileMain ( lpTemp ) );
#endif
}


/* This is the main receive-function which can optionally be threaded */
unsigned long SMDI_ReceiveFileMain ( LPVOID lpStart  )
{
  SMDI_FileTransmissionInfo ftiTemp;
  SMDI_TransmissionInfo tiTemp;
  SMDI_SampleHeader shTemp;
  unsigned long dwTemp;

  memcpy (
    &ftiTemp,
    lpStart,
    sizeof(ftiTemp) );
  memcpy (
    &tiTemp,
    (void *) ( (unsigned long) lpStart + sizeof(ftiTemp) ),
    sizeof(tiTemp) );
  memcpy (
    &shTemp,
    (void *) ( (unsigned long) lpStart + sizeof(ftiTemp) + sizeof(tiTemp) ),
    sizeof(shTemp) );

  ftiTemp.lpTransmissionInfo = &tiTemp;
  tiTemp.lpSampleHeader = &shTemp;

  free ( lpStart );

  dwTemp = SMDI_InitFileSampleReception ( &ftiTemp );
  if ( dwTemp == SMDIM_TRANSFERACKNOWLEDGE )
    {
      dwTemp = SMDIM_DATAPACKET;
      while ( dwTemp == SMDIM_DATAPACKET )
	{
	  dwTemp = SMDI_FileSampleReception ( &ftiTemp );
	  if (ftiTemp.lpCallBackProcedure != NULL)
	    (*ftiTemp.lpCallBackProcedure) ( &ftiTemp, ftiTemp.dwUserData );
	}
    }

  if ( ftiTemp.lpReturnValue != NULL )
    *ftiTemp.lpReturnValue = dwTemp;

  return ( dwTemp );
}


__declspec(dllexport) unsigned long
SMDI_ReceiveFile ( SMDI_FileTransfer * lpFileTransfer )
{
  SMDI_FileTransmissionInfo * ftiTemp;
  SMDI_TransmissionInfo     * tiTemp;
  SMDI_SampleHeader         * shTemp;
  unsigned long dwThreadID;
  void * lpTemp;
  SMDI_FileTransfer FileTransfer;

  /* Getting mem for the structures */
  ftiTemp = malloc ( sizeof(*ftiTemp) + sizeof(*tiTemp) + sizeof(*shTemp) );
  lpTemp = ftiTemp;
  tiTemp  = (void *) ((DWORD)ftiTemp + (DWORD)sizeof(*ftiTemp));
  shTemp  = (void *) ((DWORD)ftiTemp + (DWORD)sizeof(*ftiTemp) + (DWORD)sizeof(*tiTemp));

  /* Setting default values for the supplied structure */
  FileTransfer.dwStructSize = sizeof(FileTransfer);
  FileTransfer.HA_ID = 0;
  FileTransfer.SCSI_ID = 0;
  FileTransfer.dwSampleNumber = 0;
  FileTransfer.lpFileName = NULL;
  FileTransfer.dwFileType = FT_WAV;
  FileTransfer.bAsync = FALSE;
  FileTransfer.lpCallback = NULL;
  FileTransfer.dwUserData = 0;
  FileTransfer.lpReturnValue = NULL;

  /* Getting the supplied structure */
  memcpy ( &FileTransfer, 
    lpFileTransfer, 
    ((*lpFileTransfer).dwStructSize > sizeof(FileTransfer)) ? sizeof(FileTransfer) : (*lpFileTransfer).dwStructSize );

  /* Initializing structures */
  (*ftiTemp).dwStructSize = sizeof ( *ftiTemp );
  (*tiTemp).dwStructSize = sizeof ( *tiTemp );
  (*shTemp).dwStructSize = sizeof ( *shTemp );

  (*tiTemp).dwSampleNumber = FileTransfer.dwSampleNumber;
  (*tiTemp).HA_ID = FileTransfer.HA_ID;
  (*tiTemp).SCSI_ID = FileTransfer.SCSI_ID;

  (*ftiTemp).dwFileType = FileTransfer.dwFileType;
  (*ftiTemp).lpReturnValue = FileTransfer.lpReturnValue;
  strcpy ( ((*ftiTemp).cFileName), FileTransfer.lpFileName );
  (*ftiTemp).lpCallBackProcedure = FileTransfer.lpCallback;
  (*ftiTemp).dwUserData = FileTransfer.dwUserData;

  /* Setting references to each other */
  (*ftiTemp).lpTransmissionInfo = tiTemp;
  (*tiTemp).lpSampleHeader = shTemp;

  if ( FileTransfer.lpReturnValue != NULL ) *(FileTransfer.lpReturnValue) = -1;

#ifdef _win32_
  if (FileTransfer.bAsync==TRUE)
    {
      CreateThread ( NULL, 
			     0, 
			     (unsigned long (__stdcall *)(void *)) SMDI_ReceiveFileMain, 
			     lpTemp, 
			     0, 
			     &dwThreadID );
      __sleep(100);
      return ( -1 );
    } else
      return ( SMDI_ReceiveFileMain ( lpTemp ) );
#endif
#ifdef _linux_
  if (FileTransfer.bAsync==TRUE)
    {
      pthread_create ( &dwThreadID,
		       NULL,
		       (void*) SMDI_ReceiveFileMain,
		       lpTemp );
      return ( -1 );
    } else
      return ( SMDI_ReceiveFileMain ( lpTemp ) );
#endif
}


#ifdef _win32_
__declspec(dllexport)
BOOL __stdcall DllEntryPoint ( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
  SetLastError ( 0 );
  return TRUE;
}

__declspec(dllexport)
BOOL __stdcall LibMain ( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
  SetLastError ( 0 );
  return TRUE;
}
#endif



#ifdef _linux_
void machdep_sys_sendmsg ( void ) {}
void machdep_sys_recvmsg ( void ) {}
#endif

