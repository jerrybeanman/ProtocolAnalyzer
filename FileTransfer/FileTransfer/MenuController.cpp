/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	MenuController.c - For initialization of menu item handlers and filling default parameters
--
-- PROGRAM:		Protocol Analyzer
--
-- FUNCTIONS:	void FindDialogItems()
--				void EnableGUI(BOOL isEnable)
--				void AppendToStatus(HWND handle, char * buffer)
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- NOTES: 
--------------------------------------------------------------------------------------------------------------------*/
#include "MenuController.h"
HWND hwnd;			/* Handle to main window			*/
HWND hDlg;			/* Handle to dialog box				*/
HWND hServer;		/* Handle to server radiobutton		*/
HWND hClient;		/* Handle to clietn radiobutton		*/			
HWND hIP;			/* Handle to IP editfield			*/
HWND hTCP;			/* Handle to TCP radiobutton		*/
HWND hUDP;			/* Handle to UDP radiobutton		*/
HWND hInputType;	/* Handle to Input dropdown list	*/
HWND hProgress;		/* Handle to progress bar			*/
HWND hHost;			/* Handle to Hostnam editfield		*/
HWND hPort;			/* Handle to port number editfield  */			
HWND hPSize;		/* Handle to packet size editfield  */
HWND hPNum;			/* Handle to number of packets		*/
HWND hStatus;		/* Handle to status text field		*/
HWND hFilename;		/* Handle to file name text field	*/
HWND hOpenFile;		/* Handle to open file button		*/
HWND hSendFile;		/* Handle to send file button		*/
HWND hConnect;		/* Handle to connect button			*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	FindDialogItems
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void FindDialogItems()
--
-- RETURNS: void
--
-- NOTES: Initializes menu item handlers 
--------------------------------------------------------------------------------------------------------------------*/
void FindDialogItems()
{
	hServer		= GetDlgItem(hDlg, IDC_SERVER);
	hClient		= GetDlgItem(hDlg, IDC_CLIENT);
	hIP			= GetDlgItem(hDlg, IDC_IP);
	hTCP		= GetDlgItem(hDlg, IDC_TCP);
	hUDP		= GetDlgItem(hDlg, IDC_UPD);
	hPort		= GetDlgItem(hDlg, IDC_PORTNUMBER);
	hInputType	= GetDlgItem(hDlg, IDC_InputType);
	hProgress	= GetDlgItem(hDlg, IDC_PROGRESS);
	hHost		= GetDlgItem(hDlg, IDC_Host);
	hPSize		= GetDlgItem(hDlg, IDC_PSIZE);
	hPNum		= GetDlgItem(hDlg, IDC_PNUM);
	hStatus		= GetDlgItem(hDlg, IDC_STATUS);
	hFilename	= GetDlgItem(hDlg, IDC_FILENAME);
	hSendFile	= GetDlgItem(hDlg, IDC_SENDFILE);
	hOpenFile	= GetDlgItem(hDlg, IDC_FILE);
	hConnect	= GetDlgItem(hDlg, IDC_SEND);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	FillDefaultValues
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void FillDefaultValues()
--
-- RETURNS: void
--
-- NOTES: Fill in some default values for the ease of troubleshooting 
--------------------------------------------------------------------------------------------------------------------*/
void FillDefaultValues()
{
	/* Fill in input type items */
	SendMessage(hInputType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"Host Name");
	SendMessage(hInputType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"IP Address");

	/* Fill in some default values here */
	SetWindowText(hFilename, FILE_NAME);
	SetWindowText(hPort, PORT_NUMBER);
	SetWindowText(hPSize, PACKET_SIZE);
	SetWindowText(hPNum, SEND_TIMES);
	SetWindowText(hIP, IP_ADDRESS);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	EnableGUI
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void EnableGUI(BOOL isEnable)
--
-- RETURNS: void
--
-- NOTES: Enable and disable menu items depending on which mode is checked (Server or Client)
--------------------------------------------------------------------------------------------------------------------*/
void EnableGUI(BOOL isEnable)
{
	SetWindowText(hConnect, (isEnable) ? "Send Packets" : "Start Server");
	EnableWindow(hIP, isEnable);
	EnableWindow(hPSize, isEnable);
	EnableWindow(hInputType, isEnable);
	EnableWindow(hHost, isEnable);
	EnableWindow(hPNum, isEnable);
	EnableWindow(hFilename, isEnable);
	EnableWindow(hOpenFile, isEnable);
	EnableWindow(hSendFile, isEnable);
	EnableWindow(hFilename, isEnable);
	CurrentMode = (isEnable) ? CLIENT_MODE : SERVER_MODE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	AppendToStatus
--
-- DATE:		Febuary 6th, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Ruoqi Jia
--
-- PROGRAMMER:	Ruoqi Jia
--
-- INTERFACE:	void AppendToStatus(HWND handle, char * buffer)
--
-- RETURNS: void
--
-- NOTES: Wrapper function to update string into a edit field 
--------------------------------------------------------------------------------------------------------------------*/
void AppendToStatus(HWND handle, char * buffer)
{
	int index = GetWindowTextLength(handle);
	SetFocus(handle); // set focus
	SendMessageA(handle, EM_SETSEL, (WPARAM)index, (LPARAM)index); // set selection - end of text
	SendMessageA(handle, EM_REPLACESEL, 0, (LPARAM)buffer); // append!
}

