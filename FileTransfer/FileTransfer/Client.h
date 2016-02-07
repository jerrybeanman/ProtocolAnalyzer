#pragma once
#include "Global.h" 
void ClientManager(WPARAM wParam);
void Client();
DWORD WINAPI ClientThread(LPVOID lpParameter);
void SendInitialMessage(LPSOCKET_INFORMATION SOCKET_INFO, char * PacketSize, char * SendTimes);
void SendPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize);
std::string MakeInitMessage(char * PacketSize, char * SendTimes);
std::string GetDummyPacket(const int size);
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * msg, DWORD PacketSize);