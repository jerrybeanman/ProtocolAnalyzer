#include "Server.h"
#define BUFSIZE 2
SOCKET						AcceptSocket;				/* Socket for accepting clients in TCP		*/
char						str[526];					
char *						StrBuff = str; 				/* Display message buffer for status text	*/
WSAEVENT					AcceptEvent;				/* Dummy event								*/
BOOL						EndOfTransmission = FALSE;	/* Indicates EOT							*/
TRANSMISSION_INFORMATION	TransInfo;					/* Pack statistics for one tranmission		*/
FILE *						fp;							/* File descriptor for writing packets		*/
WSAEVENT					TimerEvent;
void ServerManager(WPARAM wParam)
{
	/* Check which protocol is being selected */
	CurrentProtocol = (IsDlgButtonChecked(hDlg, IDC_TCP) == BST_CHECKED) ? TCP : UDP;

	/* Connect button is pressed */
	if (LOWORD(wParam) == IDC_SEND && HIWORD(wParam) == BN_CLICKED)
	{
		if(fp != NULL)
			fclose(fp);
		WSACleanup();
		SetWindowText(hStatus, "Server Initialized...\n");
		Server();
	}
}

void Server()
{
	WSADATA		wsaData;			/* Session info						*/
	SOCKET		ListenSocket;		/* Socket for handling connection	*/
	SOCKADDR_IN InternetAddr;		/* For handling internet addresses	*/
	HANDLE		ServerThreadHandle;	/* Thread handle for server thread	*/
	HANDLE		AcceptThreadHandle;	/* Thread handle for accept thread	*/
	DWORD		AcceptThreadID;		/* Thread ID for accept thread		*/
	DWORD		ServerThreadID;		/* Thread ID for worker thread		*/
	DWORD		Ret;				/* Return value for session info	*/
	/* Create a WSA v2.2 session */
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		sprintf(StrBuff, "WSAStartup() failed with error %d\n", Ret);
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* Create socket for listening */
	if ((ListenSocket = socket(AF_INET, (CurrentProtocol == TCP) ? SOCK_STREAM : SOCK_DGRAM,
		(CurrentProtocol == TCP) ? IPPROTO_TCP : IPPROTO_UDP)) == INVALID_SOCKET)
	{
		sprintf(StrBuff, "WSASocket() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* Initialize address structure */
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(DEFAULT_PORT);

	/* Bind address to the listening socket */
	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr,
		sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		sprintf(StrBuff, "bind() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* listens for only 1 connection */
	if (CurrentProtocol == TCP && listen(ListenSocket, 1))
	{
		sprintf(StrBuff, "listen() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* Create dummy event object */
	if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		sprintf(StrBuff, "WSACreateEvent() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* If TCP: Create thread to accept connections*/
	if (CurrentProtocol == TCP &&
		(AcceptThreadHandle = CreateThread(NULL, 0, AcceptThread, (LPVOID)ListenSocket,
			0, &AcceptThreadID)) == NULL)
	{
		sprintf(StrBuff, "CreateThread() on AcceptThread failed with error %d\n", GetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* If TCP: Create thread to service accepted connections */
	if (CurrentProtocol == TCP &&
		(ServerThreadHandle = CreateThread(NULL, 0, TCPThread, (LPVOID)AcceptEvent,
			0, &ServerThreadID)) == NULL)
	{
		sprintf(StrBuff, "CreateThread() on ServerThread failed with error %d\n", GetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* If UDP: Create thread for recieving UDP packets */
	if (CurrentProtocol == UDP &&
		(ServerThreadHandle = CreateThread(NULL, 0, UDPThread, (LPVOID)ListenSocket,
			0, &ServerThreadID)) == NULL)
	{
		sprintf(StrBuff, "CreateThread() on ServerThread failed with error %d\n", GetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
}

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{

	AppendToStatus(hStatus, "Accepting connections...\n");

	/* Waits for connection */
	AcceptSocket = accept(SOCKET(lpParameter), NULL, NULL);

	AppendToStatus(hStatus, "Connection Accepted!\n");

	/* Signal the event, which is being blocked in ServerThread */
	if (WSASetEvent(AcceptEvent) == FALSE)
	{
		sprintf(StrBuff, "WSASetEvent() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI UDPThread(LPVOID lpParameter)
{
	LPSOCKET_INFORMATION	SocketInfo;		/* Socket information for a packet			*/
	DWORD					RecvBytes;		/* Actual bytes recived from recvfrom()		*/			
	DWORD					TimerThreadID;
	
	TimerEvent = WSACreateEvent();

	/* Open empty file for writing */
	fp = fopen("ouput.txt", "w");

	/* Create socket information struct to associate with the acepted socket */
	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		sprintf(StrBuff, "GlobalAlloc() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return FALSE;
	}

	SocketInfo->Socket = (SOCKET)lpParameter;
	
	/* Initialize socket info struc		*/
	FillSockInfo(SocketInfo);

	AppendToStatus(hStatus, "Reading...\n");

	/* Query for an initial message containing the transmission info */
	GetInitialMessage(SocketInfo);

	/* Start counting system timer */
	GetSystemTime(&TransInfo.StartTimeStamp);

	CreateThread(NULL, 0, TimerThread, (LPVOID)SocketInfo, 0, &TimerThreadID);

	while (TRUE)
	{
		/* Post an asynchrounous recieve request, supply ServerRoutine as the completion routine function */
		if (S_UDPRecieve(SocketInfo, TRUE, &RecvBytes) == FALSE)
		{
			sprintf(StrBuff, "recvfrom() failed with error %d\n", GetLastError());
			AppendToStatus(hStatus, StrBuff);
			return FALSE;
		}

		/* Update tranmission info value */
		UpdateTransmission(&TransInfo, RecvBytes, SocketInfo);

		/* Signal packet read to the Timer thread */
		WSASetEvent(TimerEvent);

		/* Last Packet recieved */
		if (SocketInfo->Buffer[0] == '\0' || RecvBytes == 0 || RecvBytes == UINT_MAX)
		{
			if (SocketInfo->BytesRECV > 0)
				TransInfo.PacketsRECV++;
			break;
		}

	}
	AppendToStatus(hStatus, "Ending Server Thread\n");

	/* End system timer and print out transmission info */
	GetSystemTime(&TransInfo.EndTimeStamp);
	PrintTransmission(&TransInfo);

	/* CLose the socket */
	closesocket(SocketInfo->Socket);

	/* Free socket and close session */
	GlobalFree(SocketInfo);
	memset(&TransInfo, 0, sizeof(TransInfo));
	WSACleanup();

	return TRUE;
}

DWORD WINAPI TCPThread(LPVOID lpParameter)
{
	DWORD					Flags = 0;		/* Flag for WSARecv						*/
	LPSOCKET_INFORMATION	SocketInfo;		/* Accepting Socket information			*/
	WSAEVENT				EventArray[1];	/* Accept event to wait on				*/
	DWORD					Index;			/* Event index							*/

	fp = fopen("ouput.txt", "w");			/* Open empty file for writing			*/

	EventArray[0] = AcceptEvent;			/* Save accept event in the event array */

	while (TRUE)
	{
		while (TRUE)
		{
			/* Wait for accept() to signal an event from TCP_SERVER() */
			Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

			if (Index == WSA_WAIT_FAILED)
			{
				sprintf(StrBuff, "WSASetEvent() failed with error %d\n", WSAGetLastError());
				AppendToStatus(hStatus, StrBuff);
				return FALSE;
			}

			/* Ignore IO events from the ServerRoutine() */
			if (Index != WAIT_IO_COMPLETION)
			{
				/* An accept call event is ready */
				break;
			}

			/* Indicates that last packet has been recieved from ServerRoutine() */
			if (Index == WAIT_IO_COMPLETION && EndOfTransmission)
			{
				if (SocketInfo->BytesRECV > 0)
					TransInfo.PacketsRECV++;
				AppendToStatus(hStatus, "Ending Server Thread\n");	
				EndOfTransmission = FALSE;
				closesocket(SocketInfo->Socket);			/* Close current socket				*/
				memset(&TransInfo, 0, sizeof(TransInfo));	/* Zero out transmission struct		*/
				WSACleanup();								/* Close Winsock Session			*/
				fclose(fp);									/* Close file descriptor			*/
				return TRUE;
			}
		}

		/* Set the event to non-signaled state */
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);


		/* Create socket information struct to associate with the acepted socket */
		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION)))
			== NULL)
		{
			sprintf(StrBuff, "GlobalAlloc() failed with error %d\n", WSAGetLastError());
			AppendToStatus(hStatus, StrBuff);
			return FALSE;
		}

		/* Initialize socket info struc		*/
		SocketInfo->Socket = AcceptSocket;

		/* zero out overlapped structure	*/
		FillSockInfo(SocketInfo);

		AppendToStatus(hStatus, "Reading...\n");

		/* Post an asynchrounous recieve request, supply ServerRoutine as the completion routine function */
		if (S_TCPRecieve(SocketInfo, TRUE) == FALSE)
		{
			sprintf(StrBuff, "WSARecv() failed with error %d\n", WSAGetLastError());
			AppendToStatus(hStatus, StrBuff);
			return FALSE;
		}
	}
	/* Free memory from heap */
	GlobalFree(SocketInfo);
	return TRUE;
}

void CALLBACK ServerRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	LPSOCKET_INFORMATION SI;				/* Pointer to the socket info struct			*/
	SI = (LPSOCKET_INFORMATION)Overlapped;	/* Cast Overlapped struct to SocketInfo struct	*/

	/* Check for error */
	if (Error != 0)
	{
		sprintf(StrBuff, "I/O operation failed with error %d\n", Error);
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* If no bytes recieved or sent */
	if (BytesTransferred == 0 || SI->Buffer[0] == '\0')
	{
		GetSystemTime(&TransInfo.EndTimeStamp);
		AppendToStatus(hStatus, "Closing Socket\n");
		EndOfTransmission = TRUE;

		PrintTransmission(&TransInfo);
		return;
	}

	/* Indicates a first packet arrival */
	if (TransInfo.PacketSize == 0)
	{
		/* Store packet size and expected packets */
		sscanf(SI->DataBuf.buf, "%d.%d", &TransInfo.PacketSize, &TransInfo.PacketsExpected);

		sprintf(StrBuff, "\nExpected packet size: %d, Expected number of packets: %d\n\n",
			TransInfo.PacketSize, TransInfo.PacketsExpected);
		AppendToStatus(hStatus, StrBuff);
		GetSystemTime(&TransInfo.StartTimeStamp);
	}
	else
	{
		UpdateTransmission(&TransInfo, BytesTransferred, SI);
	}

	/* Post an asynchrounous recieve request, supply ServerRoutine as the completion routine function */
	if (S_TCPRecieve(SI, TRUE) == FALSE)
	{
		sprintf(StrBuff, "WSARecv() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
}

void GetInitialMessage(LPSOCKET_INFORMATION SocketInfo)
{
	DWORD RecvBytes;
	/*Get initial packet from the client, should contain information on transmission */
	if (S_UDPRecieve(SocketInfo, FALSE, &RecvBytes) == FALSE)
	{
		sprintf(StrBuff, "recvfrom() failed with error %d\n", GetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	sscanf(SocketInfo->DataBuf.buf, "%d.%d", &TransInfo.PacketSize, &TransInfo.PacketsExpected);
	sprintf(StrBuff, "\nExpected packet size: %d, Expected number of packets: %d\n\n",
		TransInfo.PacketSize, TransInfo.PacketsExpected);
	AppendToStatus(hStatus, StrBuff);
}

void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO)
{
	ZeroMemory((&SOCKET_INFO->Overlapped), sizeof(WSAOVERLAPPED));
	SOCKET_INFO->DataBuf.len = DATA_BUFSIZE;
	SOCKET_INFO->DataBuf.buf = SOCKET_INFO->Buffer;
}

DWORD WINAPI TimerThread(LPVOID lpParameter)
{
	LPSOCKET_INFORMATION SOCKET_INFO;
	WSAEVENT e[1];
	e[0] = TimerEvent;
	SOCKET_INFO = (LPSOCKET_INFORMATION)lpParameter;
	while (TRUE)
	{
		if (WSAWaitForMultipleEvents(1, e, FALSE, 100, FALSE) == WSA_WAIT_TIMEOUT)
		{
			closesocket(SOCKET_INFO->Socket);
			return FALSE;
		}
		WSAResetEvent(e[0]);
	}
	return TRUE;
}