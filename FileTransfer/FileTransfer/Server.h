#pragma once
#include "Global.h"
void ServerManager(WPARAM wParam);
void Server();
DWORD WINAPI AcceptThread(LPVOID lpParameter);
DWORD WINAPI TCPThread(LPVOID lpParameter);
DWORD WINAPI UDPThread(LPVOID lpParameter);
void CALLBACK ServerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO);
void GetInitialMessage(LPSOCKET_INFORMATION SocketInfo);