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
#pragma once
#include "Global.h"

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
BOOL S_TCPSend(LPSOCKET_INFORMATION SOCKET_INFO);

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
BOOL S_UDPSend(LPSOCKET_INFORMATION SOCKET_INFO, SOCKADDR_IN * Server);

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
BOOL S_TCPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK);

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
BOOL S_UDPRecieve(LPSOCKET_INFORMATION SOCKET_INFO, BOOL isCALLBACK, DWORD * RecvBytes);

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
BOOL S_TCPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr);

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
BOOL S_UDPConnect(SOCKET WritingSocket, SOCKADDR_IN * InternetAddr);

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
void UpdateTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, DWORD BytesRecieved, LPSOCKET_INFORMATION SOCKET_INFO);

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
void PrintTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO);

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	WriteTransmission
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void WriteTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, char * FileName)
--
-- RETURNS: void
--
-- NOTES: Prints the transmission information values to a file
--------------------------------------------------------------------------------------------------------------------*/
void WriteTransmission(LPTRANSMISSION_INFORMATION TRANS_INFO, char * FileName);

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
long delay(SYSTEMTIME t1, SYSTEMTIME t2);