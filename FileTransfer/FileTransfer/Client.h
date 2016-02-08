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
#pragma once
#include "Global.h" 

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
void ClientManager(WPARAM wParam);

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
void Client();

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
DWORD WINAPI ClientThread(LPVOID lpParameter);

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
void SendInitialPacket(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize, DWORD SendTimes);

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
void SendFile(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize);

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
void SendDummyPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize);

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
void SendLastPacket(LPSOCKET_INFORMATION SOCKET_INFO);

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
std::string MakeInitialPacket(DWORD PacketSize, DWORD SendTimes);

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
std::string MakeDummyPacket(const int size);

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
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * msg, DWORD PacketSize);

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
int	OpenFile();