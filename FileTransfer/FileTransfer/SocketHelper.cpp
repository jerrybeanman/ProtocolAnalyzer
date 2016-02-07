#include "SocketHelper.h"
BOOL S_TCPSend(LPSOCKET_INFORMATION SOCKET_INFO)
{
	/* Post a message back to the socket for aknowledgement */
	if (WSASend(SOCKET_INFO->Socket,	/* Accepted socket						*/
		&(SOCKET_INFO->DataBuf),		/* Pointer to WSABUF					*/
		1,								/* Only 1 WSABUF						*/
		&SOCKET_INFO->BytesSEND,		/* Bytes that are sent					*/
		0,								/* No behavior to modify				*/
		&(SOCKET_INFO->Overlapped),		/* Pointer to overlapped struct			*/
		NULL)							/* No completion routine				*/
		== SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL S_UDPSend(LPSOCKET_INFORMATION SOCKET_INFO, SOCKADDR_IN * Server)
{
	if (sendto(SOCKET_INFO->Socket,	/* Accepted socket						*/
		(SOCKET_INFO->DataBuf.buf),		/* Pointer to WSABUF					*/
		SOCKET_INFO->DataBuf.len,				/* Only 1 WSABUF						*/
		0,							/* Bytes that are sent					*/
		(SOCKADDR *)Server,
		sizeof(*Server))			/* No completion routine				*/
		< 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL S_TCPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK)
{
	DWORD RecvBytes;
	DWORD Flags = 0;
	if (WSARecv(SOCKET_INFO->Socket,		/* Accepted socket					*/
		&(SOCKET_INFO->DataBuf),			/* Pointer to WSABUF				*/
		1,									/* Only one WSABUF					*/
		&RecvBytes,							/* Total bytes that is recieved		*/
		&Flags,								/* No behavior to modify			*/
		&(SOCKET_INFO->Overlapped),			/* Pointer to overlapped structure	*/
		isCALLBACK ? ServerRoutine : NULL)	/* Pointer to completion routine	*/
		== SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL S_UDPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK, DWORD * RecvBytes)
{

	DWORD Flags = 0;
	SOCKADDR_IN  client;
	int client_len = sizeof(client);

	if ((*RecvBytes = recvfrom(SOCKET_INFO->Socket,		/* Accepted socket					*/
		(SOCKET_INFO->Buffer),							/* Pointer to WSABUF				*/
		DATA_BUFSIZE,									/* Only one WSABUF					*/
		Flags,											/* Total bytes that is recieved		*/
		(SOCKADDR *)&client,
		&client_len)) 
		< 0)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL S_TCPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
{

	if (WSAConnect(WritingSocket,			/* Socket for writing			*/
		(struct sockaddr *)InternetAddr,	/* Pointer to sockaddr struct	*/
		sizeof(*InternetAddr),				/* Size of sockaddr struct		*/
		0,									/* No caller data				*/
		0,									/* No FLOWSPEC struct needed	*/
		0,
		NULL)
		!= 0)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL S_UDPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
{
	InternetAddr->sin_family = AF_INET;
	InternetAddr->sin_port = htons(0);
	InternetAddr->sin_addr.s_addr = htonl(INADDR_ANY);

	/* Bind address to the listening socket */
	if (bind(WritingSocket, (PSOCKADDR)InternetAddr,
		sizeof(*InternetAddr)) == SOCKET_ERROR)
	{
		sprintf(StrBuff, "bind() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return FALSE;
	}
	return TRUE;
}

void UpdateTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, DWORD BytesRecieved, LPSOCKET_INFORMATION SOCKET_INFO)
{
	if (TRANS_INFO->PacketSize == 0)
		return;

	SOCKET_INFO->BytesRECV += BytesRecieved;
	unsigned int Result = (unsigned int)(SOCKET_INFO->BytesRECV / TRANS_INFO->PacketSize);
	if (Result > TRANS_INFO->PacketsRECV)
	{
		sprintf(StrBuff, "Packet recieved! Size: %d\n", strlen(SOCKET_INFO->Buffer));
		AppendToStatus(hStatus, StrBuff);
		TRANS_INFO->PacketsRECV += Result - TRANS_INFO->PacketsRECV;
		fprintf(fp, SOCKET_INFO->Buffer);
	}
}

void PrintTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO)
{
	AppendToStatus(hStatus, "**************************************************\n");
	sprintf(StrBuff, "Starting Time Stamp: %d\n", TRANS_INFO->StartTimeStamp.wMilliseconds);
	AppendToStatus(hStatus, StrBuff);
	sprintf(StrBuff, "Ending Time Stamp: %d\n", TRANS_INFO->StartTimeStamp.wMilliseconds);
	AppendToStatus(hStatus, StrBuff);
	sprintf(StrBuff, "Delay: %d\n", delay(TRANS_INFO->StartTimeStamp, TRANS_INFO->EndTimeStamp));
	AppendToStatus(hStatus, StrBuff);
	sprintf(StrBuff, "Packet size: %d\n", TRANS_INFO->PacketSize);
	AppendToStatus(hStatus, StrBuff);
	sprintf(StrBuff, "Packets Expected: %d\n", TRANS_INFO->PacketsExpected);
	AppendToStatus(hStatus, StrBuff);
	sprintf(StrBuff, "Packets Recieved: %d\n", TRANS_INFO->PacketsRECV);
	AppendToStatus(hStatus, StrBuff);
	AppendToStatus(hStatus, "**************************************************\n");
}

// Compute the delay between tl and t2 in milliseconds
long delay(SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) * 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}