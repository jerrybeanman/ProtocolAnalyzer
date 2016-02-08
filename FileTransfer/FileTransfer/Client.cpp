#include "Client.h"
SOCKADDR_IN				InternetAddr;		/* For handling internet addresses	*/
BOOL					isSendFile;
void ClientManager(WPARAM wParam)
{
	/* Check which protocol is being selected */
	CurrentProtocol = (IsDlgButtonChecked(hDlg, IDC_TCP) == BST_CHECKED) ? TCP : UDP;
	if (LOWORD(wParam) == IDC_SEND && HIWORD(wParam) == BN_CLICKED)
	{
		isSendFile = FALSE;
		WSACleanup();
		sprintf(StrBuff, "%s Client Initialized...\n", CurrentProtocol == TCP ? "TCP" : "UDP");
		SetWindowText(hStatus, StrBuff);
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
			WSACleanup();
			isSendFile = TRUE;
			sprintf(StrBuff, "%s Client Initialized for sending file...\n",CurrentProtocol == TCP ? "TCP" : "UDP");
			SetWindowText(hStatus, StrBuff);
			Client();
		}
	}
}
 
void Client()
{
	char ip[256];
	char port[256];
	WSADATA					wsaData;			/* Session info						*/
	SOCKET					WritingSocket;		/* Socket for handling connection	*/
	HANDLE					ClientThreadHandle;
	DWORD					ClientThreadID;
	SOCKADDR_IN				ClientAddr;
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
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = inet_addr(ip);
	InternetAddr.sin_port = htons(atoi(port));


	/* Connect based on which protocol is currently selected */
	if (CurrentProtocol == TCP && S_TCPConnect(WritingSocket, &InternetAddr) == FALSE)
	{
		sprintf(StrBuff, "WSAConnect() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return;
	}
	else
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

DWORD WINAPI ClientThread(LPVOID lpParameter)
{
	SOCKET					WritingSocket;
	LPSOCKET_INFORMATION	SocketInfo;
	CHAR					PacketSize[8];
	CHAR					SendTimes[8];

	/* Retrieve values from window */
	GetWindowText(hPSize, PacketSize, 8);
	GetWindowText(hPNum, SendTimes, 8);

	WritingSocket = (SOCKET)lpParameter;

	/* Create socket information struct to associate with the acepted socket */
	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION)))
		== NULL)
	{
		sprintf(StrBuff, "GlobalAlloc() failed with error %d\n", WSAGetLastError());
		AppendToStatus(hStatus, StrBuff);
		return FALSE;
	}

	/* Initialize socket info struc	*/
	SocketInfo->Socket = WritingSocket;

	if (isSendFile)
	{
		/* Calculate number of packets that will be sent */
		fseek(fp, 0L, SEEK_END);
		int tmp = ceil((double)(ftell(fp) / (double)atoi(PacketSize)));
		fseek(fp, 0L, SEEK_SET);

		SendInitialPacket(SocketInfo, atoi(PacketSize), tmp);
		SendFile(SocketInfo, atoi(PacketSize));
	}
	else
	{
		SendInitialPacket(SocketInfo, atoi(PacketSize), atoi(SendTimes));
		/* transmit dummy packets */
		SendPackets(SocketInfo, atoi(SendTimes), atoi(PacketSize));
	}

	/* Send last packet to indicate end of transmission */
	SendLastPacket(SocketInfo);

	/* Wait a little bit for the server to process incoming packets before closing the socket */
	Sleep(((atoi(PacketSize) / 10)));
	closesocket(SocketInfo->Socket);

	return TRUE;
}

void SendInitialPacket(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize, DWORD SendTimes)
{
	/* zero out overlapped structure	*/
	std::string buffer = (char *)MakeInitMessage(PacketSize, SendTimes).c_str();
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

void SendPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize)
{

	/* zero out overlapped structure	*/
	std::string buffer = (char *)GetDummyPacket(PacketSize).c_str();
	FillSockInfo(SOCKET_INFO, &buffer, PacketSize);

	for (int i = 0; i < Total; i++)
	{

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
	AppendToStatus(hStatus, "End of Transmission...Closing session\n");
}

std::string MakeInitMessage(DWORD PacketSize, DWORD SendTimes)
{
	std::string message;
	std::ostringstream sstream;
	sstream << PacketSize << "." << SendTimes;
	message = sstream.str();
	return message;
}

std::string GetDummyPacket(const int size)
{
	std::string packet;
	for (int i = 0; i < size; i++)
		packet += 'z';
	return packet;
}


void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * buffer, DWORD PacketSize)
{
	/* zero out overlapped structure	*/
	ZeroMemory((&SOCKET_INFO->Overlapped), sizeof(WSAOVERLAPPED));
	SOCKET_INFO->BytesRECV = 0;
	SOCKET_INFO->BytesSEND = 0;
	SOCKET_INFO->DataBuf.len = PacketSize;
	SOCKET_INFO->DataBuf.buf = (char *)buffer->c_str();
}

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

void SendFile(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize)
{
	std::string tmp;
	DWORD	FBytesRead;
	char *	pbuf = (char *)malloc(PacketSize);
	FillSockInfo(SOCKET_INFO, &tmp, PacketSize);
	while (!feof(fp))
	{
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
	free(pbuf);
}