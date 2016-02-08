#pragma once
#include "Global.h" 
void ClientManager(WPARAM wParam);
void Client();
DWORD WINAPI ClientThread(LPVOID lpParameter);
void SendInitialPacket(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize, DWORD SendTimes);
void SendPackets(LPSOCKET_INFORMATION SOCKET_INFO, DWORD Total, DWORD PacketSize);
void SendLastPacket(LPSOCKET_INFORMATION SOCKET_INFO);
std::string MakeInitMessage(DWORD PacketSize, DWORD SendTimes);
std::string GetDummyPacket(const int size);
void FillSockInfo(LPSOCKET_INFORMATION SOCKET_INFO, std::string * msg, DWORD PacketSize);
int		OpenFile();
void SendFile(LPSOCKET_INFORMATION SOCKET_INFO, DWORD PacketSize);