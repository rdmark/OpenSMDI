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

#ifndef __aspi_h__
#define __aspi_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _win32_

#ifndef LPVOID
#define LPVOID void*
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef WORD
#define WORD unsigned short
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef BYTE
#define BYTE unsigned char
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
#define __declspec(dllexport) ;
#endif

unsigned char ASPI_GetDevType ( unsigned char ha_id, unsigned char id );
BOOL ASPI_TestUnitReady ( unsigned char ha_id, unsigned char id );
unsigned char ASPI_Check ( void );
BOOL ASPI_Send ( unsigned char ha_id, unsigned char id, void * buffer, unsigned long size );
unsigned long ASPI_Receive ( unsigned char ha_id,
                             unsigned char id, 
			     void * buffer, 
			     unsigned long size );
void ASPI_RescanPort ( unsigned char ha_id );
void ASPI_InquireDevice ( char result[], unsigned char ha_id, unsigned char id );

#ifdef __cplusplus
}
#endif

#endif

