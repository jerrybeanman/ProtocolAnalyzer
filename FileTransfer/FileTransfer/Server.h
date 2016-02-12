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
--
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
#pragma once
#include "Global.h"

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
void ServerManager(WPARAM wParam);

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
void Server();

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
DWORD WINAPI AcceptThread(LPVOID lpParameter);

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
DWORD WINAPI UDPThread(LPVOID lpParameter);

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
DWORD WINAPI TCPThread(LPVOID lpParameter);

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
void CALLBACK ServerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

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
void GetInitialMessage(LPSOCKET_INFORMATION SocketInfo);

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
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO);

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
DWORD WINAPI TimerThread(LPVOID lpParameter);

DWORD WINAPI CircularIO(LPVOID lpParameter);