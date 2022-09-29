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

#include <windows.h>
#include <stdio.h>
#include "wnaspi32.h"


/*
  Since MS VC++ compiles something like 
    while ( blabla == 123 );
  into an endless loop even though blabla _is_ altered by another DLL,
  it is necessary to do something like 
    while ( blabla == 123 ) dummyFunc ( blabla);
  so that VC++ actually compiles it correctly.
  Sorry for that, but I don't really know how to avoid that.
*/
void dummyFunc ( BYTE testByte )
{
  testByte++;
  testByte--;
}


unsigned char ASPI_GetDevType (unsigned char ha_id, unsigned char id)
{
  SRB_GDEVBlock DInfo;

  DInfo.SRB_Cmd	= SC_GET_DEV_TYPE;
  DInfo.SRB_HaId = ha_id;
  DInfo.SRB_Flags = 0;
  DInfo.SRB_Hdr_Rsvd = 0;
  DInfo.SRB_Target = id;
  DInfo.SRB_Lun	= 0;

  SendASPI32Command( (LPSRB)&DInfo );

  while ( DInfo.SRB_Status == SS_PENDING ) dummyFunc(DInfo.SRB_Status);

  return DInfo.SRB_DeviceType;
}


BOOL ASPI_TestUnitReady ( unsigned char ha_id, unsigned char id )
{
  SRB_ExecSCSICmd MySRB;
  unsigned char result;

  MySRB.SRB_Cmd = SC_EXEC_SCSI_CMD;
  MySRB.SRB_HaId = ha_id;
  MySRB.SRB_Flags = 0; //SRB_DIR_IN;
  MySRB.SRB_Hdr_Rsvd = 0;
  MySRB.SRB_Target = id;
  MySRB.SRB_Lun	= 0;
  MySRB.SRB_BufLen = 1;
  MySRB.SRB_SenseLen = SENSE_LEN;
  MySRB.SRB_BufPointer = &result;
  MySRB.SRB_CDBLen = 6;
  MySRB.CDBByte[0] = 0x00;
  MySRB.CDBByte[1] = 0;
  MySRB.CDBByte[2] = 0;
  MySRB.CDBByte[3] = 0;
  MySRB.CDBByte[4] = 0;
  MySRB.CDBByte[5] = 0;
  MySRB.CDBByte[6] = 0;
  MySRB.CDBByte[7] = 0;
  MySRB.CDBByte[8] = 0;
  MySRB.CDBByte[9] = 0;
  MySRB.CDBByte[10] = 0;
  MySRB.CDBByte[11] = 0;
  MySRB.CDBByte[12] = 0;
  MySRB.CDBByte[13] = 0;
  MySRB.CDBByte[14] = 0;
  MySRB.CDBByte[15] = 0;

  SendASPI32Command ( (LPSRB) &MySRB );
  while ( MySRB.SRB_Status == SS_PENDING ) dummyFunc(MySRB.SRB_Status);

  if ( (result & 0x3e) == 0 ) { return TRUE; } else { return FALSE; }
}

unsigned char ASPI_Check(void)
{
  unsigned long ASPIStatus;

  ASPIStatus = GetASPI32SupportInfo ();
  switch ( HIBYTE(LOWORD(ASPIStatus)) )
    {
    case SS_COMP:
      return ( LOBYTE(LOWORD(ASPIStatus)) );
    default:
      return 0;
    }
}

BOOL ASPI_Send ( unsigned char ha_id, unsigned char id, void * buffer, unsigned long size )
{
  SRB_ExecSCSICmd MyCmd;

  /* Send "SMDI Sample Header Request" */
  MyCmd.SRB_Cmd = SC_EXEC_SCSI_CMD;
  MyCmd.SRB_HaId = ha_id;
  MyCmd.SRB_Flags = SRB_DIR_OUT;
  MyCmd.SRB_Hdr_Rsvd = 0;
  MyCmd.SRB_Target = id;
  MyCmd.SRB_Lun = 0;
  MyCmd.SRB_BufLen = size;
  MyCmd.SRB_SenseLen = SENSE_LEN;
  MyCmd.SRB_BufPointer = buffer;
  MyCmd.SRB_CDBLen = 6;
  MyCmd.CDBByte[0] = 0x0a; /* SEND */
  MyCmd.CDBByte[1] = 0;
  MyCmd.CDBByte[2] = (unsigned char) ((size & 0x00ff0000) >> 16);
  MyCmd.CDBByte[3] = (unsigned char) ((size & 0x0000ff00) >> 8);
  MyCmd.CDBByte[4] = (unsigned char) ((size & 0x000000ff));
  MyCmd.CDBByte[5] = 0;
  MyCmd.CDBByte[6] = 0;
  MyCmd.CDBByte[7] = 0;
  MyCmd.CDBByte[8] = 0;
  MyCmd.CDBByte[9] = 0;
  MyCmd.CDBByte[10] = 0;
  MyCmd.CDBByte[11] = 0;
  MyCmd.CDBByte[12] = 0;
  MyCmd.CDBByte[13] = 0;
  MyCmd.CDBByte[14] = 0;
  MyCmd.CDBByte[15] = 0;

  SendASPI32Command ( (LPSRB) &MyCmd );
  while ( MyCmd.SRB_Status == SS_PENDING ) dummyFunc(MyCmd.SRB_Status);

  return TRUE;
}

unsigned long ASPI_Receive ( unsigned char ha_id,
                             unsigned char id, 
			     void * buffer, 
			     unsigned long size )
{
  SRB_ExecSCSICmd MyCmd;

  /* Send "SMDI Sample Header Request" */
  MyCmd.SRB_Cmd = SC_EXEC_SCSI_CMD;
  MyCmd.SRB_HaId = ha_id;
  MyCmd.SRB_Flags = SRB_DIR_IN;
  MyCmd.SRB_Hdr_Rsvd = 0;
  MyCmd.SRB_Target = id;
  MyCmd.SRB_Lun = 0;
  MyCmd.SRB_BufLen = size;
  MyCmd.SRB_SenseLen = SENSE_LEN;
  MyCmd.SRB_BufPointer = buffer;
  MyCmd.SRB_CDBLen = 6;
  MyCmd.CDBByte[0] = 0x08; /* RECEIVE */
  MyCmd.CDBByte[1] = 0;
  MyCmd.CDBByte[2] = (unsigned char) ((size & 0x00ff0000) >> 16);
  MyCmd.CDBByte[3] = (unsigned char) ((size & 0x0000ff00) >> 8);
  MyCmd.CDBByte[4] = (unsigned char) (size & 0x000000ff);
  MyCmd.CDBByte[5] = 0;
  MyCmd.CDBByte[6] = 0;
  MyCmd.CDBByte[7] = 0;
  MyCmd.CDBByte[8] = 0;
  MyCmd.CDBByte[9] = 0;
  MyCmd.CDBByte[10] = 0;
  MyCmd.CDBByte[11] = 0;
  MyCmd.CDBByte[12] = 0;
  MyCmd.CDBByte[13] = 0;
  MyCmd.CDBByte[14] = 0;
  MyCmd.CDBByte[15] = 0;

  SendASPI32Command ( (LPSRB) &MyCmd );
  while ( MyCmd.SRB_Status == SS_PENDING ) dummyFunc(MyCmd.SRB_Status);

  return (MyCmd.SRB_BufLen);
}


void ASPI_RescanPort ( unsigned char ha_id )
{
  SRB_RescanPort MySRB;

  MySRB.SRB_Cmd      = 7;
  MySRB.SRB_HaId     = ha_id;
  MySRB.SRB_Flags    = 0;
  MySRB.SRB_Hdr_Rsvd = 0;
  SendASPI32Command ( &MySRB );
  while ( MySRB.SRB_Status == SS_PENDING) dummyFunc(MySRB.SRB_Status);
}

void ASPI_InquireDevice ( char result[], unsigned char ha_id, unsigned char id )
{
  SRB_ExecSCSICmd MySRB;

  MySRB.SRB_Cmd = SC_EXEC_SCSI_CMD;
  MySRB.SRB_HaId = ha_id;
  MySRB.SRB_Flags = SRB_DIR_IN;
  MySRB.SRB_Hdr_Rsvd = 0;
  MySRB.SRB_Target = id;
  MySRB.SRB_Lun = 0;
  MySRB.SRB_BufLen = 96;
  MySRB.SRB_SenseLen = SENSE_LEN;
  MySRB.SRB_BufPointer = result;
  MySRB.SRB_CDBLen = 6;
  MySRB.CDBByte[0] = 0x12;
  MySRB.CDBByte[1] = 0;
  MySRB.CDBByte[2] = 0;
  MySRB.CDBByte[3] = 0;
  MySRB.CDBByte[4] = 32;
  MySRB.CDBByte[5] = 0x12;
  MySRB.CDBByte[6] = 0;
  MySRB.CDBByte[7] = 0;
  MySRB.CDBByte[8] = 0;
  MySRB.CDBByte[9] = 0;
  MySRB.CDBByte[10] = 0;
  MySRB.CDBByte[11] = 0;
  MySRB.CDBByte[12] = 0;
  MySRB.CDBByte[13] = 0;
  MySRB.CDBByte[14] = 0;
  MySRB.CDBByte[15] = 0;

  SendASPI32Command ( (LPSRB) &MySRB );
  while ( MySRB.SRB_Status == SS_PENDING ) dummyFunc(MySRB.SRB_Status);
}

