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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#include <errno.h>

#include <scsi/sg.h>
#include <scsi/scsi.h>

#define BYTE unsigned char
#define DWORD unsigned long
#define WORD unsigned short
#define FALSE 0
#define TRUE 1
#define MAX_PATH 260
#define HIBYTE(w) ((BYTE) (((WORD) (w) >> 8) & 0xff))
#define HIWORD(l) ((WORD) (((DWORD) (l) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE) (w))
#define LOWORD(l) ((WORD) (l))
typedef int BOOL;
#define CALLBACK __stdcall
#define __declspec(dllexport) ;  // For compatibility with
                                 // win32 DLL-exports
#define FARPROC void*
typedef void *LPVOID;


unsigned char ASPI_Check ( void )
{
  return 1;
}

BOOL ASPI_RescanPort ( void )
{
  return TRUE;
}

void ASPI_GetDevNameByID ( char cResult[], unsigned char ha_id, unsigned char id )
{
  sprintf ( cResult, "/dev/sg%c", id+'0' );
}

void ASPI_Receive ( unsigned char ha_id, unsigned char id, void * buffer, unsigned long size )
{
  int fd;
  void * lpBuf;
  char cFileName[MAX_PATH];
  struct sg_header sghead;
  unsigned char cInqCmd[256];
  int inp;
  int i;

  ASPI_GetDevNameByID ( cFileName, ha_id, id );
  fd = open ( cFileName, O_RDWR );
  if ( fd < 0 ) return;

  memset ( cInqCmd, 0, sizeof(cInqCmd) );

  sghead.pack_len = sizeof(sghead)+6;
  sghead.reply_len = sizeof(sghead)+size;
  sghead.result = 0;
  sghead.twelve_byte = 12==6;
  cInqCmd[sizeof(sghead)+0] = 0x08;
  cInqCmd[sizeof(sghead)+1] = 0x00;
  cInqCmd[sizeof(sghead)+2] = (unsigned char) ((size & 0x00ff0000) >> 16);
  cInqCmd[sizeof(sghead)+3] = (unsigned char) ((size & 0x0000ff00) >> 8);
  cInqCmd[sizeof(sghead)+4] = (unsigned char) ((size & 0x000000ff));
  cInqCmd[sizeof(sghead)+5] = 0x00;
  memcpy ( &cInqCmd, &sghead, sizeof(sghead) );
  lpBuf = malloc(6+size+sizeof(sghead));
  memcpy ( lpBuf, &cInqCmd, sizeof(sghead)+6 );

  write ( fd, lpBuf, sizeof(sghead)+6 );
  read ( fd, lpBuf, sizeof(sghead)+size );

  memcpy ( buffer, (void *) ((long) lpBuf)+sizeof(sghead), size );

  free ( lpBuf );
  close ( fd );
}

void ASPI_Send ( unsigned char ha_id, unsigned char id, void * buffer, unsigned long size )
{
  int fd;
  void * lpBuf;
  char cFileName[MAX_PATH];
  struct sg_header sghead;
  unsigned char cInqCmd[256];

  ASPI_GetDevNameByID ( cFileName, ha_id, id );
  fd = open ( cFileName, O_RDWR );
  if ( fd < 0 ) return;

  memset ( cInqCmd, 0, sizeof(cInqCmd) );

  sghead.pack_len = sizeof(sghead)+6+size;
  sghead.reply_len = sizeof(sghead);
  sghead.result = 0;
  cInqCmd[sizeof(sghead)+0] = 0x0a;
  cInqCmd[sizeof(sghead)+1] = 0x00;
  cInqCmd[sizeof(sghead)+2] = (unsigned char) ((size & 0x00ff0000) >> 16);
  cInqCmd[sizeof(sghead)+3] = (unsigned char) ((size & 0x0000ff00) >> 8);
  cInqCmd[sizeof(sghead)+4] = (unsigned char) ((size & 0x000000ff));
  cInqCmd[sizeof(sghead)+5] = 0x00;
  memcpy ( &cInqCmd, &sghead, sizeof(sghead) );

  lpBuf = malloc(6+size+sizeof(sghead));
  memcpy ( lpBuf, &cInqCmd, sizeof(sghead)+6 );
  memcpy ( (void *) ((long) lpBuf)+sizeof(sghead)+6, buffer, size );
  write ( fd, lpBuf, sizeof(sghead)+6+size );
  read ( fd, lpBuf, sizeof(sghead) );

  free ( lpBuf );
  close ( fd );
}

void ASPI_InquireDevice ( unsigned char * result, unsigned char ha_id, unsigned char id )
{
  int fd;
  char cFileName[MAX_PATH];
  struct sg_header sghead;
  unsigned char cInqCmd[256];

  ASPI_GetDevNameByID ( cFileName, ha_id, id );
  fd = open ( cFileName, O_RDWR );
  if ( fd < 0 ) return;

  sghead.pack_len = 6+sizeof(sghead);
  sghead.reply_len = 96;
  sghead.result = 0;
  cInqCmd[sizeof(sghead)+0] = 0x12;
  cInqCmd[sizeof(sghead)+1] = 0x00;
  cInqCmd[sizeof(sghead)+2] = 0x00;
  cInqCmd[sizeof(sghead)+3] = 0x00;
  cInqCmd[sizeof(sghead)+4] = 96;
  cInqCmd[sizeof(sghead)+5] = 0x00;
  memcpy ( &cInqCmd, &sghead, sizeof(sghead) );

  write (fd, &cInqCmd, sizeof(sghead)+6);
  read  (fd, &cInqCmd, sizeof(sghead)+96);
  memcpy ( result, &cInqCmd[sizeof(sghead)], 96 );
  
  close ( fd );
  close ( fd );
}

unsigned char ASPI_GetDevType ( unsigned char ha_id, unsigned char id )
{
  unsigned char cDevType;
  unsigned char inquire[256];

  ASPI_InquireDevice ( &inquire[0], ha_id, id );
  cDevType = inquire[0] & 0x1f;
  return (cDevType);
}

BOOL ASPI_TestUnitReady ( unsigned char ha_id, unsigned char id )
{
  int fd;
  char cFileName[MAX_PATH];
  struct sg_header sghead;
  unsigned char cInqCmd[256];
  unsigned char result;
  int i;

  ASPI_GetDevNameByID ( cFileName, ha_id, id );
  fd = open ( cFileName, O_RDWR );
  if ( fd < 0 ) return FALSE;

  sghead.pack_len = 6+sizeof(sghead);
  sghead.reply_len = sizeof(sghead);
  sghead.result = 0;
  cInqCmd[sizeof(sghead)+0] = 0x00;
  cInqCmd[sizeof(sghead)+1] = 0x00;
  cInqCmd[sizeof(sghead)+2] = 0x00;
  cInqCmd[sizeof(sghead)+3] = 0x00;
  cInqCmd[sizeof(sghead)+4] = 0x00;
  cInqCmd[sizeof(sghead)+5] = 0x00;
  memcpy ( &cInqCmd, &sghead, sizeof(sghead) );

  write (fd, &cInqCmd, sizeof(sghead)+6);
  read  (fd, &sghead, sizeof(sghead));

  result = sghead.sense_buffer[12];
  close ( fd );
  return (result == 0);
}

