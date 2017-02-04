//#include "stdafx.h"
//***************************************************************************
// Describtion：RS232 SERIAL COM PORT COMMUNICATION
//***************************************************************************
#include "RS232.h"
#include "other.h"
CRS232 rs232;
//***************************************************************************
// Describtion：CONSTRUCTOR/DESTRUCTOR
//***************************************************************************

CRS232::CRS232() 
{
	::OutputDebugString("CRS232::CRS232()");
	m_hCOM = NULL;
}
CRS232::~CRS232() 
{
	::OutputDebugString("CRS232::~CRS232()");
	if(NULL != m_hCOM)
	{
		::CloseHandle(m_hCOM);
		m_hCOM = NULL;
	}
}
//***************************************************************************
// Describtion：RS232 OPEN COM PORT
//***************************************************************************
INT CRS232::Open(RS232_CONFIG stConfig)
{
	DCB             stDCB;
	COMMTIMEOUTS    stTimeout;
	CHAR            szFileName[16] = {0x00};

	// Set Port String and Number
	sprintf(szFileName, "\\\\.\\COM%d", stConfig.iCOM);
	m_hCOM = ::CreateFile(	szFileName,
							GENERIC_READ | GENERIC_WRITE,
							0,					//shared
							NULL,				//security
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							NULL);

	if(INVALID_HANDLE_VALUE == m_hCOM)
	{
		::CloseHandle(m_hCOM);
		return RS232_HANDLE_FAIL;
	}
	if(!SetCommMask(m_hCOM, EV_RXCHAR))
	{
		Close();
		return RS232_OPEN_FAIL;
	}
	// Set Buffer Size
	if(!::SetupComm(m_hCOM, 4096, 4096))
	{
		Close();
		return RS232_OPEN_FAIL;
	}

	::PurgeComm(m_hCOM, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	::ZeroMemory(&m_OverlappedRead, sizeof(OVERLAPPED));
	::ZeroMemory(&m_OverlappedWrite, sizeof(OVERLAPPED));
	m_OverlappedRead.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_OverlappedWrite.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// Set no time for port
	stTimeout.ReadIntervalTimeout           = MAXDWORD;
	stTimeout.ReadTotalTimeoutConstant      = 0;
	stTimeout.ReadTotalTimeoutMultiplier    = 0;
	stTimeout.WriteTotalTimeoutConstant     = 0;
	stTimeout.WriteTotalTimeoutMultiplier   = 0;
	if(!::SetCommTimeouts(m_hCOM, &stTimeout))
	{
		Close();
		return RS232_OPEN_FAIL;
	}

	::GetCommState(m_hCOM, &stDCB);

	// Set port parameters
	stDCB.DCBlength				= sizeof(DCB);
	stDCB.BaudRate				= stConfig.BaudRate;	//CBR_9600
	stDCB.Parity				= stConfig.Parity;		//NOPARITY
	stDCB.ByteSize				= stConfig.ByteSize;	//8
	stDCB.StopBits				= stConfig.StopBits;	//1
	stDCB.fBinary				= stConfig.fBinary;		//1
	stDCB.fTXContinueOnXoff		= FALSE;				//TRUE
	stDCB.StopBits				= ONESTOPBIT;

	switch(stConfig.FlowControl)
	{
	case FLOW_CONTROL_OFF:
		stDCB.fOutxCtsFlow		= FALSE;
		stDCB.fOutxDsrFlow		= FALSE;
		stDCB.fDsrSensitivity	= FALSE;
		stDCB.fDtrControl		= DTR_CONTROL_ENABLE;
		stDCB.fRtsControl		= RTS_CONTROL_ENABLE;
		stDCB.fOutX				= FALSE;
		stDCB.fInX				= FALSE;
		break;
	case FLOW_CONTROL_SOFTWARE:
		stDCB.fOutxCtsFlow		= FALSE;
		stDCB.fOutxDsrFlow		= FALSE;
		stDCB.fDsrSensitivity	= FALSE;
		stDCB.fDtrControl		= DTR_CONTROL_ENABLE;
		stDCB.fRtsControl		= RTS_CONTROL_ENABLE;
		stDCB.fOutX				= TRUE;
		stDCB.fInX				= TRUE;
		break;
	case FLOW_CONTROL_HARDWARE:
		stDCB.fOutxCtsFlow		= TRUE;
		stDCB.fOutxDsrFlow		= TRUE;
		stDCB.fDsrSensitivity	= TRUE;
		stDCB.fDtrControl		= DTR_CONTROL_ENABLE;
		stDCB.fRtsControl		= RTS_CONTROL_HANDSHAKE;
		stDCB.fOutX				= FALSE;
		stDCB.fInX				= FALSE;
		break;
	default:
		break;
	}
	if(FALSE == ::SetCommState(m_hCOM, &stDCB))
	{
		::CloseHandle(m_hCOM);
		return RS232_OPEN_FAIL;
	}

	return ERROR_SUCCESS;
}
//***************************************************************************
// Describtion：RS232 CLOSE COM PORT
//***************************************************************************

INT CRS232::Close(VOID)
{
	if(INVALID_HANDLE_VALUE != m_hCOM)
	{
		//::EscapeCommFunction(m_hCOM, CLRDTR);
		//::EscapeCommFunction(m_hCOM, CLRRTS);
		::SetCommMask(m_hCOM, 0);
		::PurgeComm(m_hCOM, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		::CloseHandle(m_hCOM);
		m_hCOM = NULL;
	}
	return ERROR_SUCCESS;
}
//***************************************************************************
// Describtion：RS232 DETECT COM PORT
//***************************************************************************

INT CRS232::Detect(CONST INT iID)
{
	HANDLE	hCOM = NULL;
	CHAR	szFileName[16] = {0x00};
	sprintf(szFileName, "\\\\.\\COM%d", iID);
	hCOM = ::CreateFile(	szFileName,
							GENERIC_READ | GENERIC_WRITE,
							0,					//not shared
							NULL,				//no security
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							NULL				//template
	);

	if(INVALID_HANDLE_VALUE == hCOM) 
	{
		::CloseHandle(hCOM);
		return RS232_HANDLE_FAIL;
	}

	// Detected COM then Close Handle
	::CloseHandle(hCOM);
	return ERROR_SUCCESS;
}
//***************************************************************************
// Describtion：RS232 WRITE STRING
//***************************************************************************

INT CRS232::WriteString(CONST CHAR *pcCommand)
{
	DWORD dwBytesWrite = 0;
	DBGTRACE("SHOW SEND COMMAND:");
    DBGTRACE((char *)pcCommand);
	Clear();
	if(FALSE == ::WriteFile(m_hCOM, pcCommand, (INT)strlen(pcCommand), &dwBytesWrite, &m_OverlappedWrite))
	{
		if(ERROR_IO_PENDING == ::GetLastError())
		{
			if(::WaitForSingleObject(m_OverlappedWrite.hEvent, INFINITE))
			{
				dwBytesWrite = -1;
			}
			else
			{
				::GetOverlappedResult(m_hCOM, &m_OverlappedWrite, &dwBytesWrite, FALSE);
			}
		}
	}
	return ERROR_SUCCESS;
}
//***************************************************************************
// Describtion：RS232 READ STRING
//***************************************************************************

INT CRS232::ReadString(CHAR *pcBuffer)
{
	DWORD 		dwBytesRead = 0;
	DWORD 		dwCommError = 0;
	CHAR 		szInBuffer[40960] = {0x00};
	COMSTAT 	CS;

	::ClearCommError(m_hCOM, &dwCommError, &CS);

	if(0 == CS.cbInQue)
	{
		return RS232_READ_FAIL;
	}

	if(CS.cbInQue > sizeof(szInBuffer))
	{
		::PurgeComm(m_hCOM, PURGE_RXCLEAR);
		return RS232_READ_FAIL;
	}

	dwBytesRead = (DWORD)CS.cbInQue;

	if(FALSE == ::ReadFile(m_hCOM, szInBuffer, CS.cbInQue, &dwBytesRead, &m_OverlappedRead))
	{
		if(ERROR_IO_PENDING == ::GetLastError())
		{
			if(::WaitForSingleObject(m_OverlappedRead.hEvent, INFINITE))
			{
				//dwBytesRead = -1;
				return RS232_READ_FAIL;     //如果fail的話就return read fail
			}
			else
			{
				::GetOverlappedResult(m_hCOM, &m_OverlappedRead, &dwBytesRead, FALSE);
				szInBuffer[dwBytesRead] = '\0';
				strcpy(pcBuffer, szInBuffer);
			}
		}
	}
	else
	{
		if(0 < dwBytesRead)
		{
			pcBuffer[dwBytesRead] = '\0';
			strcpy(pcBuffer, szInBuffer);
		}
	}
	return ERROR_SUCCESS;
}
//***************************************************************************
// Describtion：RS232 WRITE BYTE
//***************************************************************************

INT CRS232::WriteByte(RS232_AT_COMMAND &cmd)
{
	DWORD dwWriteBytes = 0;

	if(FALSE == ::WriteFile(m_hCOM, cmd.szCommand, cmd.dwLength, &dwWriteBytes, &m_OverlappedWrite))
	{
		if(ERROR_IO_PENDING == ::GetLastError())
		{
			if(::WaitForSingleObject(m_OverlappedWrite.hEvent, INFINITE))
			{
				dwWriteBytes = -1;
				return 1;
			}
			else
			{
				::GetOverlappedResult(m_hCOM, &m_OverlappedWrite, &dwWriteBytes, FALSE);
			}
		}else
		{
			return 1;
		}
	}

	return ERROR_SUCCESS;
}
//***************************************************************************
// Describtion：RS232 READ BYTE
//***************************************************************************

INT CRS232::ReadByte(CHAR *pcBuffer)
{
	DWORD 		dwReadBytes = 0;
	DWORD 		dwCommError = 0;
	CHAR 		szInBuffer[1024] = {0x00};
	COMSTAT 	CS;
	
	::ClearCommError(m_hCOM, &dwCommError, &CS);

	if(0 == CS.cbInQue)
	{
		return RS232_READ_FAIL;
	}
	if(CS.cbInQue > sizeof(szInBuffer))
	{
		PurgeComm(m_hCOM, PURGE_RXCLEAR);
		return RS232_READ_FAIL;
	}

	dwReadBytes = (DWORD)CS.cbInQue;

	if(FALSE == ::ReadFile(m_hCOM, szInBuffer, CS.cbInQue, &dwReadBytes, &m_OverlappedRead))
	{
		if(ERROR_IO_PENDING == ::GetLastError())
		{
			if(::WaitForSingleObject(m_OverlappedRead.hEvent, INFINITE))
			{
				dwReadBytes = -1;
			}
			else
			{
				::GetOverlappedResult(m_hCOM, &m_OverlappedRead, &dwReadBytes, FALSE);
				for(DWORD i=0; i<dwReadBytes; i++)
				{
					pcBuffer[i] = szInBuffer[i];
				}
				szInBuffer[dwReadBytes] = '\0';
			}
		}
	}
	else
	{
		for(DWORD i=0; i<dwReadBytes; i++)
		{
			pcBuffer[i] = szInBuffer[i];
		}
		szInBuffer[dwReadBytes] = '\0';
	}

	return ERROR_SUCCESS;
}

VOID CRS232::Clear(VOID)
{
	if(INVALID_HANDLE_VALUE != m_hCOM)
		::PurgeComm(m_hCOM, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
}
//***************************************************************************
// Describtion：END
//***************************************************************************
