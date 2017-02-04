/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  ATS Team Engineer, HHD ODM BU-ATS Dept.
 *  ASUSTek Computer Inc. (C) 2005-2008
 *
 ******************************************************************************/

/*******************************************************************************
 *==============================================================================
 * Filename:
 * ---------
 *  RS232.h
 *
 * Project:
 * --------
 *  ALL
 *
 * Description:
 * ------------
 *  Serial COM Port Communication
 *
 * Author:
 * -------
 *  Achigo_Liu (ASUS#15059/PEGATRON#LA0802543)
 *
 *  Rev 1.0.0	Nov 09 2005
 *  Rev 1.0.1	Aug 28 2008
 *==============================================================================
 *******************************************************************************/

#ifndef _RS232_H
#define _RS232_H

#include "windows.h"
#include "stdio.h"
//#include "CommonAPI.h"

namespace RS232
{

typedef enum _RS232_RETURN_STATUS {
	RS232_HANDLE_FAIL		= 0x01,
	RS232_OPEN_FAIL			= 0x02,
	RS232_DETECT_FAIL		= 0x03,
	RS232_READ_FAIL			= 0x04,
	RS232_WRITE_FAIL		= 0x05,
	RS232_PARSER_FAIL		= 0x06,
} RS232_RETURN_STATUS;

typedef enum _RS232_FLOW_CONTROL {
	FLOW_CONTROL_OFF		= 0,
	FLOW_CONTROL_SOFTWARE	= 1,
	FLOW_CONTROL_HARDWARE	= 2,
} RS232_FLOW_CONTROL;

typedef struct _RS232_CONFIG {
	IN	BOOL	bEnable;
	IN	INT		iCOM;
	IN	DWORD 	BaudRate;
	IN	BYTE 	Parity;
	IN	BYTE 	ByteSize;
	IN	DWORD 	fBinary;
	IN	DWORD	FlowControl;
	IN	BYTE	StopBits;
	IN	CHAR	Description[32];
} RS232_CONFIG;

typedef struct _RS232_AT_COMMAND {
	IN	CHAR	szCommand[256];
	IN	DWORD	dwLength;
} RS232_AT_COMMAND;

class CRS232
{
protected:
	HANDLE 				m_hCOM;
	OVERLAPPED 			m_OverlappedRead;
	OVERLAPPED 			m_OverlappedWrite;

public:

	CRS232();
	virtual ~CRS232();

public:

	INT Open(RS232_CONFIG stConfig);
	INT Close(VOID);
	INT Detect(CONST INT iID);
	INT WriteString(CONST CHAR *pcCommand);
	INT ReadString(CHAR *pcBuffer);
	INT WriteByte(RS232_AT_COMMAND &cmd);
	INT ReadByte(CHAR *pcBuffer);
	VOID Clear(VOID);
};

}
using namespace RS232;
//---------------------------------------------------------------------------
extern CRS232 rs232;
//---------------------------------------------------------------------------
#endif /* #pragma once */
