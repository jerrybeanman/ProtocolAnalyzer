/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	Server.c - Provides the logical functionalities of the server sided service using both TCP and UDP
--							recieve packets or datagrams 
--
-- PROGRAM:		Protocol Analyzer
--
-- FUNCTIONS:	void ServerManager(WPARAM wParam)
--				void Server()
--				DWORD WINAPI AcceptThread(LPVOID lpParameter)
--				DWORD WINAPI UDPThread(LPVOID lpParameter)
--				DWORD WINAPI TCPThread(LPVOID lpParameter)
--				void CALLBACK ServerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--				void GetInitialMessage(LPSOCKET_INFORMATION SocketInfo)
--				void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO)
--				DWORD WINAPI TimerThread(LPVOID lpParameter)
--
--				
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia 

-- PROGRAMMER:	Ruoqi Jia
--
-- NOTES: Contains the logical functionalities of the server sided service for TCP and UDP connections.

--			General FLow: The ClientManager() listens for user button clicks when in server mode. When the connect button
--				is pressed it transitions into Client(), which handles all the main logic for establishing setups and 
--				connections for both TCP and UDP, depending on which one is selected. A thread is created for both protocols
--				to service incoming connections and traffics.
--				UDP thread: 
--					Starts streaming for data right away from any source, it waits for a initial message from the client 
--						that contains control information for the transmission, listens for incoming datagrams, then 
--						calcualtes transmission statistics when the stream of datagrams end.
--				TCP Accept Thread:
--					It waits for incoming connection from the client, and when it a connection has been successfully 
--						established, it signals a dummy event that is being waited on in the TCP server thread.
--				TCP Server Thread:
--					Waits indefinitely for I/O events from the socket, either a successful recieve call or a client has 
--						been accepted. The TCP uses CallBack Routines to recieve incoming packets. When the transmission 
--						EOT packet has been recieved, the thread calcualtes the transmission stattistics and exit.
--
--------------------------------------------------------------------------------------------------------------------*/

#include "Server.h"
#define BUFSIZE 2
char						str[526];
SOCKET						AcceptSocket;				/* Socket for accepting clients in TCP					*/				
char *						StrBuff = str; 				/* Display message buffer for status text				*/
WSAEVENT					AcceptEvent;				/* Dummy event											*/
BOOL						EndOfTransmission = FALSE;	/* Indicates EOT										*/
TRANSMISSION_INFORMATION	TransInfo;					/* Pack statistics for one tranmission					*/
FILE *						fp;							/* File descriptor for writing packets					*/
WSAEVENT					TimerEvent;					/* Event that is checked periodically for I/O status	*/
WSAEVENT					CircularEvent;				/* Event that is checked everytime UDP recieves data	*/
CircularBuffer CircularBuff;	/* Circular buffer to prevent stack overflow on UDP recieve	*/
LARGE_INTEGER				Frequency;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	ServerManager
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void ServerManager()
--
-- RETURNS: void
--
-- NOTES: Main lobby for the server side. When the Connect button is pressed and the user is curretly in server mode,
--			it transmistions to the main server logic.
--------------------------------------------------------------------------------------------------------------------*/
void ServerManager(WPARAM wParam)
{
	/* Check which protocol is being selected */
	CurrentProtocol = (IsDlgButtonChecked(hDlg, IDC_TCP) == BST_CHECKED) ? TCP : UDP;

	/* Connect button is pressed */
	if (LOWORD(wParam) == IDC_SEND && HIWORD(wParam) == BN_CLICKED)
	{
		QueryPerformanceFrequency(&Frequency);
		memset(&TransInfo, 0, sizeof(TransInfo));

		/* If file descriptor is already open, close it*/
		if(fp != NULL)
			fclose(fp);
		
		/* CLose current session incase it's already opened */
		WSACleanup();

		sprintf(StrBuff, "%s Server Initialized...\n", CurrentProtocol == TCP ? "TCP" : "UDP");
		SetWindowText(hStatus, StrBuff);

		/* Reset progress bar */
		SendMessage(hProgress, PBM_SETPOS, 0, 0);

		Server();
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Server
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void Server()
--
-- RETURNS: void
--
-- NOTES: Main starting point of the server sided application. It sets up the corresponding socket and address 
--			structure for TCP or UDP (depending on which is selected) and creates the thread needed for them. 		
--------------------------------------------------------------------------------------------------------------------*/
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

	/* Create dummy event object for TCP */
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	AcceptThread
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	DWORD WINAPI AcceptThread(LPVOID lpParameter) 
--					LPVOID lpParamter : Pointer to the void listening socket.
--
-- RETURNS: exit code 
--
-- NOTES: Thread created for TCP to accept client. Since this is a blocking call its better to place it in a thread. 
--------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	UDPThread
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	DWORD WINAPI UDPThread(LPVOID lpParameter)
--					LPVOID lpParamter : pointer to the void accepted socket 					
--
-- RETURNS: exit code
--
-- NOTES: Thread created to service incoming UDP traffics for the server. It initializes the SOCKET_INFORMATION structure,
--			which will be used to store packet information from the client. Before calling read in a loop, the thread
--			waits for an initial control message sent from the client, and extracts the information which contains
--			the control context of that transmission. After the thread finishes reading a transmission, it calculates
--			the transmission info statistcs and display it on the screen.
--------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI UDPThread(LPVOID lpParameter)
{
	LPSOCKET_INFORMATION	SocketInfo;			/* Socket information for a packet			*/
	DWORD					RecvBytes;			/* Actual bytes recived from recvfrom()		*/			
	DWORD					TimerThreadID;		/* ID for the TimerThread					*/
	DWORD					CircularThreadID;	/* ID for circularIO thread					*/	

	/* Initialize circular buffer to be size of ten */
	CBInitialize(&CircularBuff, CIRCULAR_BUF_SIZE, sizeof(SOCKET_INFORMATION));

	/* Create a dummy WSA event for the timer thread to listens for IO events */
	TimerEvent = WSACreateEvent();
	
	CircularEvent = WSACreateEvent();

	/* Open empty file for writing */
	fp = fopen("ouput.txt", "w");

	/* Create socket information struct to associate with the acepted socket */
	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		sprintf(StrBuff, "GlobalAlloc() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return FALSE;
	}

	/* Initialize socket */
	SocketInfo->Socket = (SOCKET)lpParameter;
	
	/* Initialize socket info struc		*/
	FillSockInfo(SocketInfo);

	AppendToStatus(hStatus, "Reading...\n");

	/* Query for an initial message containing the transmission info */
	GetInitialMessage(SocketInfo);

	/* Start counting system timer */
	QueryPerformanceCounter(&TransInfo.StartTimeStamp);

	/* Create the TimerThread to keep track of missing packets */
	CreateThread(NULL, 0, TimerThread, (LPVOID)SocketInfo, 0, &TimerThreadID);

	CreateThread(NULL, 0, CircularIO, NULL, 0, &CircularThreadID);

	while (TRUE)
	{
		/* Post an asynchrounous recieve request, supply ServerRoutine as the completion routine function */
		if (S_UDPRecieve(SocketInfo, TRUE, &RecvBytes) == FALSE)
		{
			sprintf(StrBuff, "recvfrom() failed with error %d\n", GetLastError());
			AppendToStatus(hStatus, StrBuff);
			return FALSE;
		}

		/* Signal packet read to the Timer thread */
		WSASetEvent(TimerEvent);

		/* Last Packet recieved */
		if (SocketInfo->Buffer[0] == '\0' || RecvBytes == 0 || RecvBytes == UINT_MAX)
		{
			if (SocketInfo->BytesRECV > 0)
				TransInfo.PacketsRECV++;
			break;
		}

		CircularBuff.BytesRECV = RecvBytes;
		CBPushBack(&CircularBuff, SocketInfo);
		WSASetEvent(CircularEvent);
		TransInfo.PacketsRECV++;
	}
	AppendToStatus(hStatus, "Ending Server Thread\n");

	/* End system timer and print out transmission info */
	QueryPerformanceCounter(&TransInfo.EndTimeStamp);

	SendMessage(hProgress, PBM_STEPIT, 0, 0);	/* Increment progress bar */

	PrintTransmission(&TransInfo);

	/* CLose the socket */
	closesocket(SocketInfo->Socket);

	/* Free socket and close session */
	WSACleanup();

	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	TCPThread
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	DWORD WINAPI TCPThread(LPVOID lpParameter)
--
-- RETURNS: exit code
--
-- NOTES: Thread created to service incoming TCP traffics for the server. It initializes the SOCKET_INFORMATION structure,
--			which will be used to store packet information from the client. The thread waits on the Accept Thread 
--			to signal an successful client connection before it starts reading. When the event occured, WsaRecv() is 
--			called and a completion routine function is supplied to handle arriving packets.
--------------------------------------------------------------------------------------------------------------------*/
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
				/* An accept call event is signled  */
				break;
			}

			/* Indicates that last packet has been recieved from ServerRoutine() */
			if (Index == WAIT_IO_COMPLETION && EndOfTransmission)
			{
				if (SocketInfo->BytesRECV > 0)					/* If threre are remaining bytes left	*/
				{
					TransInfo.PacketsRECV += ceil(SocketInfo->BytesRECV / TransInfo.PacketSize);	
				}
				EndOfTransmission = FALSE;						/* Reset flag							*/
				closesocket(SocketInfo->Socket);				/* Close current socket					*/
				memset(&TransInfo, 0, sizeof(TransInfo));		/* Zero out transmission struct			*/
				WSACleanup();									/* Close Winsock Session				*/
				fclose(fp);										/* Close file descriptor				*/
				AppendToStatus(hStatus, "Ending Server Thread\n");
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	ServerRoutine
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void CALLBACK ServerRoutine(DWORD Error, 
--											DWORD BytesTransferred, 
--											LPWSAOVERLAPPED Overlapped, 
--											DWORD InFlags)
--					DWORD			Error				: Error code
--					DOWRD			BytesTransffered	: Total bytes recieved from packet
--					LPWSAOVERLAPPED	Overlapped			: Overlapped structure
--					DWORD			InFlags				: Modification flags 
--
-- RETURNS: void
--
-- NOTES: Callback completion routine for WSARecv when a packet has been recieved. It checks for the arrival of an 
--			control message specifying the transmission context and fills in the corresponding info, and checkes for the
--			EOT packet. The fuction recursively calls WSARecv with itself as the completion routine to read more 
--			packets.
--------------------------------------------------------------------------------------------------------------------*/
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
		AppendToStatus(hStatus, "Closing Socket\n");
		QueryPerformanceCounter(&TransInfo.EndTimeStamp);			/* Get the ending time stamp for this transmission	*/
		EndOfTransmission = TRUE;						/* Indicate end of transmission packet				*/
		PrintTransmission(&TransInfo);					/* Print out statistics								*/
		return;
	}

	/* Indicates a first packet arrival */
	if (TransInfo.PacketSize == 0)
	{
		/* Store packet size and expected packets */
		sscanf(SI->DataBuf.buf, "%d.%d", &TransInfo.PacketSize, &TransInfo.PacketsExpected);

		sprintf(StrBuff, "\nExpected packet size: %d, Expected number of packets: %d\n\n", TransInfo.PacketSize, TransInfo.PacketsExpected);
		AppendToStatus(hStatus, StrBuff);
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, TransInfo.PacketsExpected * 10));
		/* Get the starting time stamp for this transmisssion */
		QueryPerformanceCounter(&TransInfo.StartTimeStamp);
	}
	else
	{
		/* Update statistics */
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	GetInitialMessage
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void GetInitialMessage(LPSOCKET_INFORMATION SocketInfo)
--					LPSOCKET_INFORMATION	SocketInfo : socket to read from 
--
-- RETURNS: exit code
--
-- NOTES: Used by the UDP thread to wait on an initial message containing the control context of the transmission.
--			when the message is recieved, the corresponding variables are stored into the TRANSMISSION_INFORMATION structure 
--------------------------------------------------------------------------------------------------------------------*/
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

	/* Extract and store the packet size and number of packets expected into TRANSMISSSION_INFORMATION struct */
	sscanf(SocketInfo->DataBuf.buf, "%d.%d", &TransInfo.PacketSize, &TransInfo.PacketsExpected);

	/* Set progress bar range			*/
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, TransInfo.PacketsExpected * 10));

	sprintf(StrBuff, "\nExpected packet size: %d, Expected number of packets: %d\n\n", TransInfo.PacketSize, TransInfo.PacketsExpected);
	AppendToStatus(hStatus, StrBuff);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	FillSockInfo
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO)
--
-- RETURNS: void
--
-- NOTES: Initialize the SOCKET_INFORMATION structure.
--------------------------------------------------------------------------------------------------------------------*/
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO)
{

	/* Zero out the overlapped struct */
	ZeroMemory((&SOCKET_INFO->Overlapped), sizeof(WSAOVERLAPPED));

	SOCKET_INFO->DataBuf.len = DATA_BUFSIZE;
	SOCKET_INFO->DataBuf.buf = SOCKET_INFO->Buffer;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	TimerThread
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	DWORD WINAPI TimerThread(LPVOID lpParameter)
--
-- RETURNS: void
--
-- NOTES: Sets a timer for the UDP thread. If a packet didnt arrive in the given time it will timeout and close the 
--			socket. It is used for when packets are dropped due to the nature of UDP and we need to determine if there
--			are anymore incoming traffics coming in to determine the end of transmission.
--------------------------------------------------------------------------------------------------------------------*/
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

DWORD WINAPI CircularIO(LPVOID lpParameter)
{
	int c = 0;
	LPSOCKET_INFORMATION tmp = (LPSOCKET_INFORMATION)malloc(sizeof(SOCKET_INFORMATION));
	DWORD ret;
	WSAEVENT e[1];
	e[0] = CircularEvent;
	while (TRUE)
	{
		ret = WSAWaitForMultipleEvents(1, e, FALSE, 100, FALSE);
		if (ret == WSA_WAIT_TIMEOUT)
		{
			CBFree(&CircularBuff);
			free(tmp);
			fclose(fp);
			return FALSE;
		}
		if(ret != WAIT_IO_COMPLETION)
		{
			while (CircularBuff.Count != 0)
			{
				CBPop(&CircularBuff, tmp);
				SendMessage(hProgress, PBM_DELTAPOS, 10, 0);	/* Increment progress bar */
				/* Write the packet content to a output file */
				fwrite(tmp->DataBuf.buf, sizeof(char), tmp->DataBuf.len, fp);
				ResetEvent(CircularEvent);
				c++;
			}
		}
	}
	free(tmp);
	return TRUE;
}