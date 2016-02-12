/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	client.c - Provides the logical functionalities of the client sided service using both TCP and UDP 
--							to send dummy packets and file
--
-- PROGRAM:		Protocol Analyzer
--
-- FUNCTIONS:	void ClientManager(WPARAM wParam)
--				void Client()
--				DWORD WINAPI ClientThread(LPVOID lpParameter)
--				void SendInitialPacket(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize, DWORD SendTimes)
--				void SendFile(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize) 
--				void SendDummyPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize)
--				void SendLastPacket(LPSOCKET_INFORMATION SOCKET_INFO)
--				std::string MakeInitialPacket(DWORD PacketSize, DWORD SendTimes)
--				std::string MakeDummyPacket(const int size)
--				void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * buffer, DWORD PacketSize)
--				int OpenFile()
--				
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- NOTES: Contains the logical functionalities of the client sided service for TCP and UDP connections.

--			General FLow: The ClientManager() listens for user button clicks when in client mode. When the connect button
--				is pressed it transitions into Client(), which starts up a WinSock session, create a socket for either TCP
--				or UDP, and connect them to the IP address or Hostname of the server specified in the UI. 
--				A thread is created to handle sending packets to the server (TCP and UDP). An initial control message
--				is sent to specify the packet size and number of packets to send, therefore the server is able to 
--				keep track of statistics on that tranmission.
--				If no file is specified in the UI, dummy packets will be created and sent out.
--				Lastly, an EOT packet is sent out to specified the end of the tranmission after.
--
--------------------------------------------------------------------------------------------------------------------*/
#include "Client.h"
SOCKADDR_IN				InternetAddr;		/* For handling internet addresses		*/
BOOL					isSendFile;			/* Indicates that we are sending a file	*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	ClientManager
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void ClientManager(WPARAM wParam)
--					WPARAM wParam: Window message sent by the dialog box to detect clicks on the send button 
--
-- RETURNS: void
--
-- NOTES: Listens for button presses in client mode, there are three choices: "Send Packet", "Open File", and "Send File".
--			When they are pressed, it sets the corresponding boolean value to indicate wether it should send dummy 
--			packets or files, and then transitions to Client() function
--------------------------------------------------------------------------------------------------------------------*/
void ClientManager(WPARAM wParam)
{
	/* Check which protocol is being selected */
	CurrentProtocol = (IsDlgButtonChecked(hDlg, IDC_TCP) == BST_CHECKED) ? TCP : UDP;

	/* If Send Packet button is being pressed */
	if (LOWORD(wParam) == IDC_SEND && HIWORD(wParam) == BN_CLICKED)
	{
		WSACleanup();			/* Close session if its already open	*/
		isSendFile = FALSE;		/* Indicates that we are sending a file */

		sprintf(StrBuff, "%s Client Initialized...\n", CurrentProtocol == TCP ? "TCP" : "UDP");
		SetWindowText(hStatus, StrBuff);

		/* Reset progress bar */
		SendMessage(hProgress, PBM_SETPOS, 0, 0);

		Client();
	}
	/* if open file is being pressed */
	if (LOWORD(wParam) == IDC_FILE && HIWORD(wParam) == BN_CLICKED)
	{
		if (OpenFile() == -1)
			SetWindowText(hStatus, "Invalid File name\n");
	}

	/* if send file is being pressed */
	if (LOWORD(wParam) == IDC_SENDFILE && HIWORD(wParam) == BN_CLICKED)
	{
		if (fp == NULL)
		{
			SetWindowText(hStatus, "Please Open a file first\n");
		}
		else
		{
			WSACleanup();			/* Close session if its already open	*/
			isSendFile = TRUE;		/* Indicates that we are sending a file */

			sprintf(StrBuff, "%s Client Initialized for sending file...\n",CurrentProtocol == TCP ? "TCP" : "UDP");
			SetWindowText(hStatus, StrBuff);

			/* Reset progress bar */
			SendMessage(hProgress, PBM_SETPOS, 0, 0);

			Client();
		}
	}
}
 
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	Client
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void Client()
--
-- RETURNS: void
--
-- NOTES: Main starting point of the client side service for both TCP and UDP. It will establish the initial setup 
--			depending on which service is currently selected in the UI.
--				TCP:
--					- Create a socket for streaming TCP packets 
--					- Attempt to connect to the server via IP address or hostname specified in the UI
--				UDP:
--					- Create a socket for streaming UDP packets
--					- Binds an address structure to the structure for sending
--			Creates a thread for sending packets for TCP or UDP when done
--------------------------------------------------------------------------------------------------------------------*/
void Client()
{
	char					ip[256];			/* IP address for the target host	*/
	char					port[256];			/* Port number for the target host	*/
	WSADATA					wsaData;			/* Session info						*/
	SOCKET					WritingSocket;		/* Socket for handling connection	*/
	HANDLE					ClientThreadHandle;	/* Thread handle for the new thread	*/
	DWORD					ClientThreadID;		/* Thread handle ID					*/
	SOCKADDR_IN				ClientAddr;			/* Address structure to bind on		*/


	/* Create a WSA v2.2 session */
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		sprintf(StrBuff, "WSAStartup() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* Create socket for writing based on currently selected protocol */
	if ((WritingSocket = socket(AF_INET, (CurrentProtocol == TCP) ? SOCK_STREAM : SOCK_DGRAM,
		(CurrentProtocol == TCP) ? IPPROTO_TCP : IPPROTO_UDP)) == INVALID_SOCKET)
	{
		sprintf(StrBuff, "WSASocket() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	/* Initialize address structure */
	GetWindowText(hIP, ip, 256);
	GetWindowText(hPort, port, 256);
	InternetAddr.sin_family			= AF_INET;
	InternetAddr.sin_addr.s_addr	= inet_addr(ip);
	InternetAddr.sin_port			= htons(atoi(port));


	/* Connect based on which protocol is currently selected								 */
	/* If TCP: Attempt to connect to target host via IP address or Host name and Port number */
	if (CurrentProtocol == TCP && S_TCPConnect(WritingSocket, &InternetAddr) == FALSE)
	{
		sprintf(StrBuff, "WSAConnect() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
	else
	/* If UDP: Initialize and binds client address strcutre to the soccket					*/
	if (CurrentProtocol == UDP && S_UDPConnect(WritingSocket, &ClientAddr) == FALSE)
	{
		sprintf(StrBuff, "bind() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}

	sprintf(StrBuff, "Server: %s, Port: %s\n", ip, port);
	AppendToStatus(hStatus, StrBuff);

	/* Create thread to handle transmission */
	if ((ClientThreadHandle = CreateThread(NULL, 0, ClientThread, (LPVOID)WritingSocket,
		0, &ClientThreadID)) == NULL)
	{
		sprintf(StrBuff, "CreateThread() on ClientThread failed with error %d\n", GetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	ClientThread
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	DWORD WINAPI ClientThread(LPVOID lpParameter)
--					LPVOID lpParameter: Socket object for operation
--
-- RETURNS: thread exit code
--
-- NOTES: Handles transmission for the client side service for both TCP and UDP, the sending is mostly the same 
--			regardless of the protocol selected (WSASend for TCP, WSASendTo for UDP). The transmission includes three
--			steps: Initial packet to specify control message (packet size and number of packets), sending the packets
--			(either read from a file or just dummy packets), and an EOT packet to specify end of transmission.
--------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ClientThread(LPVOID lpParameter)
{
	LPSOCKET_INFORMATION	SocketInfo;			/* Socket information structure					*/
	CHAR					PacketSize[8];		/* Size of each packet							*/
	CHAR					SendTimes[8];		/* Number of times the packets will be sent		*/
	DWORD					TotalBytes;			/* Total bytes that will be sent	*/

	/* Retrieve values from window */
	GetWindowText(hPSize, PacketSize, 8);
	GetWindowText(hPNum, SendTimes, 8);

	/* Create socket information struct to associate with the acepted socket */
	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION)))
		== NULL)
	{
		sprintf(StrBuff, "GlobalAlloc() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return FALSE;
	}

	/* Initialize socket info struc		*/
	SocketInfo->Socket = (SOCKET)lpParameter;

	/* Check if we want to send a file	*/
	if (isSendFile)
	{
		/* Calculate number of packets that will be sent */
		fseek(fp, 0L, SEEK_END);
		int tmp = ceil((double)(ftell(fp) / (double)atoi(PacketSize)));
		fseek(fp, 0L, SEEK_SET);
		
		/* Set progress bar range			*/
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, tmp * 10));

		SendInitialPacket(SocketInfo, atoi(PacketSize), tmp);
		SendFile(SocketInfo, atoi(PacketSize));
	}
	/* Send dummy packets instead		*/
	else
	{

		/* Set progress bar range			*/
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, atoi(SendTimes) * 10));

		SendInitialPacket(SocketInfo, atoi(PacketSize), atoi(SendTimes));
		/* transmit dummy packets */
		SendDummyPackets(SocketInfo, atoi(SendTimes), atoi(PacketSize));
	}

	/* Send last packet to indicate end of transmission */
	SendLastPacket(SocketInfo);

	TotalBytes = atoi(PacketSize) * atoi(SendTimes);

	AppendToStatus(hStatus, "End of Transmission...Closing session\n");
	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendInitialPacket
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void SendInitialPacket(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize, DWORD SendTimes)
--					LPSOCKET_INFORMATION	SOCKET_INFO	: Socket information structure, will store the packet content
--					DWORD					PacketSize	: Size of the packet
--					DWORD					SendTimes	: Number of times the packet will be sent
--
-- RETURNS: void
--
-- NOTES: Creates a initial control message containing in order for the server to aknowledge and calculate incoming
--			traffic statistics. The message will be formatted as "PacketSize.SendTimes"
--					ex. "2000.1", Send one packet of size 2000 bytes
--------------------------------------------------------------------------------------------------------------------*/
void SendInitialPacket(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize, DWORD SendTimes)
{
	/* Retrieve the formmated packet */
	std::string buffer = (char *)MakeInitialPacket(PacketSize, SendTimes).c_str();

	/* Fill socket information with the corresponding packet content */
	FillSockInfo(SOCKET_INFO, &buffer, strlen(buffer.c_str()));

	/* Different send methods based on the current protocol */
	if ((CurrentProtocol == TCP ? S_TCPSend(SOCKET_INFO) :
		S_UDPSend(SOCKET_INFO, &InternetAddr)) == FALSE)
	{
		sprintf(StrBuff, "WSASend() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendFile
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void SendFile(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize)
--					LPSOCKET_INFORMATION	SOCKET_INFO : Socket information structure, will be used to write to the socket
--					DWORD					PaccketSize : Size of each packet
--
-- RETURNS: void
--
-- NOTES: Keep on sending the file as packets to the socket until EOF is reached.
--------------------------------------------------------------------------------------------------------------------*/
void SendFile(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize)
{
	std::string tmp;			/* empty packet for initializing SOCKET_INFO	*/						
	DWORD		FBytesRead;		/* Bytes read from fread						*/				
	char *		pbuf;			/* Pointer to buffer recieved from fread		*/

	/* Allocate memory for buffer and initialize SOCKET_INFO */
	pbuf = (char *)malloc(PacketSize);
	FillSockInfo(SOCKET_INFO, &tmp, PacketSize);

	/* Keeps reading from file */
	while (!feof(fp))
	{
		SendMessage(hProgress, PBM_DELTAPOS, 10, 0);	/* Increment progress bar */

		/* read PackeSize bytes from file */
		FBytesRead = fread(pbuf, sizeof(char), PacketSize, fp);
		SOCKET_INFO->DataBuf.buf = pbuf;
		SOCKET_INFO->DataBuf.len = FBytesRead;

		sprintf(StrBuff, "Sending File Packet... Size:  %d\n", FBytesRead);
		AppendToStatus(hStatus, StrBuff);

		/* Different send methods based on the current protocol */
		if ((CurrentProtocol == TCP ? S_TCPSend(SOCKET_INFO) :
			S_UDPSend(SOCKET_INFO, &InternetAddr)) == FALSE)
		{
			sprintf(StrBuff, "WSASend() failed with error %d\n", WSAGetLastError());
			AppendToStatus(hStatus, StrBuff);
			return;
		}
		/* zero out memory for next round */
		memset(pbuf, 0, PacketSize);
	}

	/* Free memory from heap */
	free(pbuf);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendDummyPackets
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void SendDummyPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize)
--					LPSOCKET_INFORMATION	SOCKET_INFO : Socket information structure for writing 
--					DWORD					Total		: Number of packets to send
--					DWORD					PacketSize	: Size of each packet 
--
-- RETURNS: void
--
-- NOTES: Creates a dummy packet and send it to the socket until total amount of packets to send is reached 
--------------------------------------------------------------------------------------------------------------------*/
void SendDummyPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize)
{

	/* Make a dummy packet */
	std::string buffer = (char *)MakeDummyPacket(PacketSize).c_str();

	/* Initialize SOCKET_INFO structure	with the dummy packet */
	FillSockInfo(SOCKET_INFO, &buffer, PacketSize);

	/* Keep on sending the dummy packet to the socket */
	for (int i = 0; i < Total; i++)
	{
		SendMessage(hProgress, PBM_DELTAPOS, 10, 0);	/* Increment progress bar */

		sprintf(StrBuff, "Sending Packet... Size:  %d\n", strlen(SOCKET_INFO->DataBuf.buf));
		AppendToStatus(hStatus, StrBuff);

		/* Different send methods based on the current protocol */
		if ((CurrentProtocol == TCP ? S_TCPSend(SOCKET_INFO) :
			S_UDPSend(SOCKET_INFO, &InternetAddr)) == FALSE)
		{
			sprintf(StrBuff, "WSASend() failed with error %d\n", WSAGetLastError());
			AppendToStatus(hStatus, StrBuff);
			return;
		}
	}

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	SendLastPacket
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void SendLastPacket(LPSOCKET_INFORMATION SOCKET_INFO)
--						LPSOCKET_INFORMATION SOCKET_INFO : Socket information structure for writing 
--
-- RETURNS: void
--
-- NOTES: Send an EOT packet to the socket 
--------------------------------------------------------------------------------------------------------------------*/
void SendLastPacket(LPSOCKET_INFORMATION SOCKET_INFO)
{
	/* Initialize a null packet */
	SOCKET_INFO->DataBuf.len = 1;
	SOCKET_INFO->DataBuf.buf = "\0";

	/* Post a message back to the socket to indicate end of transmission */
	if ((CurrentProtocol == TCP ? S_TCPSend(SOCKET_INFO) :
		S_UDPSend(SOCKET_INFO, &InternetAddr)) == FALSE)
	{
		sprintf(StrBuff, "WSASend() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	MakeInitialPacket
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	std::string MakeInitialPacket(DWORD PacketSize, DWORD SendTimes)
--						DWORD PacketSize : Size of packet 
--						DWORD SendTimes  : Number of times to send the packet 
--
-- RETURNS:the formateed packet content stored as a std::string 
--
-- NOTES: Create and formate a initial packet content message 
--			eg. "PacketSize.SendTimes"
--------------------------------------------------------------------------------------------------------------------*/
std::string MakeInitialPacket(DWORD PacketSize, DWORD SendTimes)
{
	std::string message;
	std::ostringstream sstream;
	sstream << PacketSize << "." << SendTimes;
	message = sstream.str();
	return message;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	MakeDummyPacket
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	std::string MakeDummyPacket(const int size)
--						const int size : Packet size of the dummy packet 
--
-- RETURNS: The dummy paccket stored as a std::string 
--
-- NOTES: Create a dummy packet full of 'z' 
--------------------------------------------------------------------------------------------------------------------*/
std::string MakeDummyPacket(const int size)
{
	std::string packet;
	for (int i = 0; i < size; i++)
		packet += 'z';
	return packet;
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
-- INTERFACE:	void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * buffer, DWORD PacketSize)
--						LPSOCKET_INFORMATION	SOCKET_INFO : Socket information structure to initialize 
--						std::string		*		buffer		: Message content that will be stored in SOCKET_INFO
--						DWORD					PacketSize  : Size of packet 
--
-- RETURNS: void
--
-- NOTES: Initialize SOCKET_INFO with the corresponding parameters 
--------------------------------------------------------------------------------------------------------------------*/
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * buffer, DWORD PacketSize)
{
	/* zero out overlapped structure	*/
	ZeroMemory((&SOCKET_INFO->Overlapped), sizeof(WSAOVERLAPPED));
	
	SOCKET_INFO->BytesRECV = 0;
	SOCKET_INFO->BytesSEND = 0;
	SOCKET_INFO->DataBuf.len = PacketSize;
	SOCKET_INFO->DataBuf.buf = (char *)buffer->c_str();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	OpenFile
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	int OpenFile()
--
-- RETURNS: success value 
--
-- NOTES: Wrapper function to open a file. File name is specified in the UI 
--------------------------------------------------------------------------------------------------------------------*/
int OpenFile()
{
	char str[32];
	GetWindowText(hFilename, str, 32);
	if ((fp = fopen(str, "r")) == NULL)
	{
		return -1;
	}
	sprintf(StrBuff, "Successfully opened file: %s\n", str);
	SetWindowText(hStatus, StrBuff);
	return 0;
}

