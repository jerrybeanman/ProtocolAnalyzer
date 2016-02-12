#ifndef _GLOBALS
#define _GLOBALS

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#define SERVER_MODE			101
#define CLIENT_MODE			102
#define TCP					103
#define UDP					104
#define IP_INPUT			105
#define HOST_INPUT			106
#define MAXBUF				256
#define DEFAULT_PORT		7000
#define DATA_BUFSIZE		60000
#define FILE_NAME			TEXT("WarAndPeace")
#define PORT_NUMBER			TEXT("7000")
#define PACKET_SIZE			TEXT("60000")
#define SEND_TIMES			TEXT("100")
#define IP_ADDRESS			TEXT("192.168.1.67")
#define CIRCULAR_BUF_SIZE	60
#include <iostream>
#include <sstream>
#include <math.h>
#include <limits.h>
#include <WinSock2.h>
#include <string>
#include "resource1.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <commctrl.h>
#include <time.h>

extern HWND hStatus;				/* Handle to status text field		*/

typedef struct _SOCKET_INFORMATION 
{
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct _TRANSMISSION_INFORMATION
{
	DWORD PacketSize;
	DWORD PacketsExpected;
	DWORD PacketsRECV;
	SYSTEMTIME StartTimeStamp;
	SYSTEMTIME EndTimeStamp;
	LPSTR ProtocolType;
} TRANSMISSION_INFORMATION, *LPTRANSMISSION_INFORMATION;

#include "MenuController.h"
#include "CircularBuffer.h"
#include "SocketHelper.h"
#include "Client.h"
#include "Server.h"

extern TRANSMISSION_INFORMATION TransInfo;
extern FILE * fp;
extern char	szFile[260];				/* buffer for file name							*/
extern DWORD CurrentMode;				/* Current mode selected (Client or Server)		*/
extern DWORD CurrentSelectedInputType;	/* Current input type selected (IP or hostname)	*/
extern DWORD CurrentProtocol;			/* Current protocol selected (TCP or UDP)		*/


extern char str[526];
extern char * StrBuff;

/* WINDOWS STUFF...*/
extern HWND hwnd;					/* Handle to main window			*/
extern HWND hDlg;					/* Handle to dialog box				*/
extern HWND hServer;				/* Handle to server radiobutton		*/
extern HWND hClient;				/* Handle to clietn radiobutton		*/
extern HWND hTCP;					/* Handle to IP editfield			*/
extern HWND hInputType;				/* Handle to TCP radiobutton		*/
extern HWND hHost;					/* Handle to UDP radiobutton		*/
extern HWND hProgress;				/* Handle to progress bar			*/
extern HWND hIP;					/* Handle to Input dropdown list	*/
extern HWND hUDP;					/* Handle to Hostnam editfield		*/
extern HWND hPort;					/* Handle to port number editfield  */
extern HWND hPSize;					/* Handle to packet size editfield  */
extern HWND hPNum;					/* Handle to number of packets		*/
extern HWND hFilename;				/* Handle to file name text field	*/
extern HWND hOpenFile;				/* Handle to open file button		*/
extern HWND hSendFile;				/* Handle to send file button		*/
extern HWND hConnect;				/* Handle to connect button			*/
extern HWND hCombA;

#endif