#ifndef _GLOBALS
#define _GLOBALS

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#define SERVER_MODE		101
#define CLIENT_MODE		102
#define TCP				103
#define UDP				104
#define IP_INPUT		105
#define HOST_INPUT		106
#define MAXBUF			256
#define DEFAULT_PORT	7000
#define DATA_BUFSIZE	100000

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
#include "SocketHelper.h"
#include "Client.h"
#include "Server.h"
extern MSG Msg;

extern TRANSMISSION_INFORMATION TransInfo;
extern FILE * fp;
extern HANDLE		hf;              /* file handle */
extern std::string	readFrom;
extern OPENFILENAME ofn;			 /* common dialog box structure */
extern HANDLE		fileReadWriteThread;
extern DWORD		fileReadWriteID;

extern char	szFile[260];     /* buffer for file name */

extern DWORD CurrentMode;
extern DWORD CurrentSelectedInputType;
extern DWORD CurrentProtocol;


extern char str[526];
extern char * StrBuff;
extern HWND hwnd;					/* owner window */
extern HWND hDlg;
extern HWND hServer;				/* owner window */
extern HWND hClient;				/* owner window */
extern HWND hTCP;					/* owner window */
extern HWND hInputType;
extern HWND hHost;
extern HWND hIP;
extern HWND hUDP;					/* owner window */
extern HWND hPort;					/* owner window */
extern HWND hPSize;					/* owner window */
extern HWND hPNum;					/* owner window */
extern HWND hStatus;
extern HWND hFilename;				/* owner window */
extern HWND hOpenFile;				/* owner window */
extern HWND hSendFile;
extern HWND hConnect;				/* owner window */
extern HWND hCombA;

#endif