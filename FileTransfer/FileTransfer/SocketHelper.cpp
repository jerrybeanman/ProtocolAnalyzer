/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	SocketHelper.c - Provides wrapper functions to communicate with the socket
--
-- PROGRAM:		Protocol Analyzer
--
-- FUNCTIONS:	BOOL S_TCPSend(LPSOCKET_INFORMATION SOCKET_INFO)
--				BOOL S_UDPSend(LPSOCKET_INFORMATION SOCKET_INFO, SOCKADDR_IN * Server)
--				BOOL S_TCPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK)
--				BOOL S_UDPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK, DWORD * RecvBytes)
--				BOOL S_TCPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
--				BOOL S_UDPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
--				void UpdateTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, DWORD BytesRecieved, LPSOCKET_INFORMATION SOCKET_INFO)
--				void PrintTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO)
--				long delay(SYSTEMTIME t1, SYSTEMTIME t2)
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- NOTES: Provides wrapper function for TCP and UDP services to communicate with a socket. Such as sending, recieving,
--			and connecting. 
--			Also provide functions to calculate statistics for a tramission 
--------------------------------------------------------------------------------------------------------------------*/
#include "SocketHelper.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	S_TCPSend
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	BOOL S_TCPSend(LPSOCKET_INFORMATION SOCKET_INFO)
--					LPSOCKET_INFORMATION SOCKET_INFO : Socket information structure for writing
--
-- RETURNS: success value 
--
-- NOTES: Writes to a socket for TCP service 
--------------------------------------------------------------------------------------------------------------------*/
BOOL S_TCPSend(LPSOCKET_INFORMATION SOCKET_INFO)
{
	/* Post a message back to the socket for aknowledgement */
	if (WSASend(SOCKET_INFO->Socket,	/* Writing socket						*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	S_UDPSend
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	BOOL S_TCPSend(LPSOCKET_INFORMATION SOCKET_INFO)
--					LPSOCKET_INFORMATION SOCKET_INFO : Socket information structure for writing
--
-- RETURNS: success value
--
-- NOTES: Writes to a socket, and the target host destination for UDP service 
--------------------------------------------------------------------------------------------------------------------*/
BOOL S_UDPSend(LPSOCKET_INFORMATION SOCKET_INFO, SOCKADDR_IN * Server)
{
	if (sendto(SOCKET_INFO->Socket,		/* Writing socket						*/
		(SOCKET_INFO->DataBuf.buf),		/* Message content						*/
		SOCKET_INFO->DataBuf.len,		/* Size of the message					*/
		0,								/* Bytes that are sent					*/
		(SOCKADDR *)Server,				/* Server socket address structure		*/
		sizeof(*Server))				/* size of the socket address structure	*/
		< 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	S_TCPRecieve
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	BOOL S_TCPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK)
--					LPSOCKET_INFORMATION SOCKET_INFO : Socket information structure to recive packets from
--					BOOL				 isCALLBACK  : Do we want completion routine or nah?
--
-- RETURNS: success value
--
-- NOTES: Recives from a socket for TCP service, also calls the completion routine function if needed 
--------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	S_UDPRecieve
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	BOOL S_UDPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK, DWORD * RecvBytes)
--					LPSOCKET_INFORMATION SOCKET_INFO : Socket information structure to recive packets from
--					BOOL				 isCALLBACK  : Do we want completion routine or nah?
--					DWORD			*	 RecvBytes   : Bytes recived 
--
-- RETURNS: success value
--
-- NOTES: Recives from a socket for TCP service, also calls the completion routine function if needed
--------------------------------------------------------------------------------------------------------------------*/
BOOL S_UDPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK, DWORD * RecvBytes)
{

	DWORD Flags = 0;
	SOCKADDR_IN  client;
	int client_len = sizeof(client);

	if ((*RecvBytes = recvfrom(SOCKET_INFO->Socket,		/* Accepted socket					*/
		(SOCKET_INFO->Buffer),							/* Message buffer to recieve		*/
		DATA_BUFSIZE,									/* Maximum data to recieve			*/
		Flags,											/* No modification					*/
		(SOCKADDR *)&client,							/* Server socket address structure		*/
		&client_len))									/* size of the socket address structure	*/
		< 0)
	{
		return FALSE;
	}
	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	S_TCPConnect
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	BOOL S_TCPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
--					SOCKET			WritingSocket : Socket to connect to
--					SOCKADDR_IN	*	InternetAddr  : Address structure that stores the target address 
--
-- RETURNS: success value
--
-- NOTES: Attempt to connect to a server
--------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	S_UDPConnect
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	BOOL S_UDPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
--					SOCKET			WritingSocket : Socket to connect to
--					SOCKADDR_IN	*	InternetAddr  : Address structure that stores the client's address
--
-- RETURNS: success value
--
-- NOTES: Not actually "connecting" to a server since its UDP. It basically initializes the client's address structure
--			and binds it to the socket 
--------------------------------------------------------------------------------------------------------------------*/
BOOL S_UDPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr)
{

	/* Initalize client address for UDP */
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	UpdateTransmission
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void UpdateTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, DWORD BytesRecieved, LPSOCKET_INFORMATION SOCKET_INFO)
--					LPTRANSMISSION_INFORMATION TRANS_INFO : Contain statistics on current transmission 
--					DWORD BytesRecieved : Total bytes recieved from S_UDPRecv or S_TCPRecv
--					LPSOCKETINFORMATION SOCKET_INFO : Socket information structure 
--
-- RETURNS: void
--
-- NOTES: Updates the transmission information structture for statistics 
--------------------------------------------------------------------------------------------------------------------*/
void UpdateTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, DWORD BytesRecieved, LPSOCKET_INFORMATION SOCKET_INFO)
{
	/* If a reciev call failed */
	if (BytesRecieved == UINT_MAX)
		return;

	/* Append BytresRECV value */
	SOCKET_INFO->BytesRECV += BytesRecieved;

	/* Check if we have enough bytes to fill up a packet */
	int Result = (int)(SOCKET_INFO->BytesRECV / TRANS_INFO->PacketSize);

	sprintf(StrBuff, "Data recieved! Size: %d\n", BytesRecieved);
	AppendToStatus(hStatus, StrBuff);

	/* If bytes recieved is greater than a packet length */
	if (Result > 0)
	{
		TRANS_INFO->PacketsRECV += Result;	/* Increment packets recieved by the maximum divider	*/
		SOCKET_INFO->BytesRECV -= Result	/* Add remaining bytes to SOCKET_INFORMATION->BytesRECV	*/		
			* TRANS_INFO->PacketSize;
	}

	/* Write the packet content to a output file */
	fprintf(fp, SOCKET_INFO->DataBuf.buf);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	PrintTransmission
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void PrintTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO)
--					LPTRANSMISSION_INFORMATION TRANS_INFO : Transmission information to print out
--
-- RETURNS: void
--
-- NOTES: Prints the transmission information values 
--------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	delay
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	long delay(SYSTEMTIME t1, SYSTEMTIME t2)
--					SYSTEMTIME t1 : Starting time
--					SYSTEMTIME t2 : Ending Time 
--
-- RETURNS: void
--
-- NOTES: Compute the delay between tl and t2 in milliseconds
--------------------------------------------------------------------------------------------------------------------*/
long delay(SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) * 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}