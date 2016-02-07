#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#define PORT 5150
#define DATA_BUFSIZE 8192
void main(void)
{
	WSABUF DataBuf;
	DWORD EventTotal 0;
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAOVERLAPPED AcceptOverlapped;
	SOCKET ListenSocket, AcceptSocket;

	// Step 1: Start Winsock and set up a listening socket


	//Step 2: Accept an inbound connection
	AcceptSocket = accept(ListenSocket, NULL, NULL);

	//Step 3: Set up an overlapped structure
	EventArray[EventTotal] = WSACreateEvent();

	ZeroMemory(&AcceptOverlapped, sizeof(WSAOVERLAPPED));
	AcceptOverlapped.hEvent = EventArray[EventTotal];

	DataBuf.len = DATA_BUFSIZE:
	DataBuf.buf = buffer;

	EventTotal++;

	//Step 4: Post a WSARecv request to begin receiving data on the socket
	WSARecv(
		AcceptSocket,
		&DataBuf,				// array of WSABUF structure 
		1,						// number of WSABUF structures
		&RecvBytes,				// total bytes recieved 
		&Flags,					// flags used to modify the fuction's behavior
		&AcceptOverlapped,		// pointer to overlapped structure
		NULL					// completion routine
		);

	//Process overlapped receives on the socket.
	while (TRUE)
	{
		//Step 5: Wait for the overlapped I/O call to complete 
		Index = WSAWaitForMultipleEvents(
			EventTotal,			// number of events to wait for
			EventArray,			// array of events
			FALSE,				// return when any event is alerted
			WSA - INFINITE,		// wait time
			FALSE				// do we want I/O completion routine?
			);

		// Index should be 0 because we have only one event handle in EventArray

		// Step 6: Reset the signaled event 
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// Step 7: Determine the status of the overlapped request
		WSAGetOverlappedResult(
			AcceptSocket,			// socket descriptor
			&AcceptOverlapped,		// WSAOVERLAPPED stucture
			&BytesTransferred,		// number of bytes that were actually transferred by send or recieve
			FALSE,					// may be set to TRUE only if the overlapped operation selected the event-based completion notification.
			&flags   				// same flags as WSARecv
			);

		// First check to see whether the peer has closed the connection, 
		// and if so, close the socket
		if (BytesTransferred == 0)
		{
			printf("Closing socket %d\n", AcceptSocket);
			closesocket(AcceptSocket);
			WSACloseEvent(EventArray[Index WSA_WAIT_EVENT_0]);
			return;
		}

		// Do something with the received data.
		// DataBuf contains the received data.


		// Step 8: Post another WSARecv() request on the socket

		Flags = 0;
		ZeroMemory(&AcceptOverlapped, sizeof(WSAOVERLAPPED));
		AcceptOverlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

		DataBuf.len = DATA - BUFSIZE;
		DataBuf.buf = Buffer;

		WSARecv(AcceptSocket,
			&DataBuf,
			1,
			&RecvBytes,
			&Flags,
			&AcceptOverlapped,
			NULL
			);
	}
}