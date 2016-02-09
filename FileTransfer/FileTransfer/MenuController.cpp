#include "MenuController.h"
HANDLE			hf;              // file handle
OPENFILENAME	ofn;			 // common dialog box structure
char			szFile[260];     /* buffer for file name */
HANDLE			fileReadWriteThread;
std::string		readFrom;
DWORD			fileReadWriteID;

HWND hwnd;						/* owner window */
HWND hDlg;
HWND hServer;					/* owner window */
HWND hClient;					/* owner window */
HWND hIP;
HWND hTCP;						/* owner window */
HWND hUDP;						/* owner window */
HWND hInputType;
HWND hHost;
HWND hPort;						/* owner window */
HWND hPSize;					/* owner window */
HWND hPNum;						/* owner window */
HWND hStatus;
HWND hFilename;					/* owner window */
HWND hOpenFile;					/* owner window */
HWND hSendFile;
HWND hConnect;					/* owner window */

void FindDialogItems()
{
	hServer		= GetDlgItem(hDlg, IDC_SERVER);
	hClient		= GetDlgItem(hDlg, IDC_CLIENT);
	hIP			= GetDlgItem(hDlg, IDC_IP);
	hTCP		= GetDlgItem(hDlg, IDC_TCP);
	hUDP		= GetDlgItem(hDlg, IDC_UPD);
	hPort		= GetDlgItem(hDlg, IDC_PORTNUMBER);
	hInputType	= GetDlgItem(hDlg, IDC_InputType);
	hHost		= GetDlgItem(hDlg, IDC_Host);
	hPSize		= GetDlgItem(hDlg, IDC_PSIZE);
	hPNum		= GetDlgItem(hDlg, IDC_PNUM);
	hStatus		= GetDlgItem(hDlg, IDC_STATUS);
	hFilename	= GetDlgItem(hDlg, IDC_FILENAME);
	hSendFile	= GetDlgItem(hDlg, IDC_SENDFILE);
	hOpenFile	= GetDlgItem(hDlg, IDC_FILE);
	hConnect	= GetDlgItem(hDlg, IDC_SEND);

	SendMessage(hInputType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"Host Name");
	SendMessage(hInputType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"IP Address");

	/* Fill in some default values here */
	SetWindowText(hFilename, "WarAndPeace");
	SetWindowText(hPort, "7000");
	SetWindowText(hPSize, "2000");
	SetWindowText(hPNum, "1");
	SetWindowText(hIP, "142.232.50.240");
}

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

void AppendToStatus(HWND handle, char * buffer)
{
	int index = GetWindowTextLength(handle);
	SetFocus(handle); // set focus
	SendMessageA(handle, EM_SETSEL, (WPARAM)index, (LPARAM)index); // set selection - end of text
	SendMessageA(handle, EM_REPLACESEL, 0, (LPARAM)buffer); // append!
}

