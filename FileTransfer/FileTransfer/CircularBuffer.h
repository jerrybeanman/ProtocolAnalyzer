#pragma once
#include "Global.h"
/* Can be any type */
struct CircularBuffer
{
	void	*	buffer;			/* data buffer								*/
	void	*	buffer_end;		/* end of data buffer						*/
	DWORD		MaxSize;		/* maximum number of items in the buffer	*/
	DWORD		Count;			/* number of items in the buffer			*/
	DWORD		ElementSize;	/* size of each item in the buffer			*/
	void	*	Front;			/* pointer to Front							*/
	void	*	Rear;			/* pointer to Rear							*/
	DWORD		BytesRECV;		/* Number of bytes recieved					*/
};


void CBInitialize(CircularBuffer * CBuff, DWORD MaxSize, DWORD ElementSize);

void CBFree(CircularBuffer * CBuff);


void CBPushBack(CircularBuffer * CBuff, const void * item);

void CBPop(CircularBuffer * CBuff, void * item);
