#include "MenuController.h"
HANDLE			hf;              // file handle
OPENFILENAME	ofn;			 // common dialog box structure
char			szFile[260];     /* buffer for file name */
HANDLE			fileReadWriteThread;
std::string		readFrom;
DWORD			fileReadWriteID;

HWND hwnd;					/* owner window */
HWND hDlg;
HWND hServer;					/* owner window */
HWND hClient;					/* owner window */
HWND hIP;
HWND hTCP;					/* owner window */
HWND hUDP;					/* owner window */
HWND hInputType;
HWND hHost;
HWND hPort;					/* owner window */
HWND hPSize;					/* owner window */
HWND hPNum;					/* owner window */
HWND hStatus;
HWND hFilename;					/* owner window */
HWND hFileIO;					/* owner window */
HWND hConnect;					/* owner window */

void FindDialogItems()
{
	hServer = GetDlgItem(hDlg, IDC_SERVER);
	hClient = GetDlgItem(hDlg, IDC_CLIENT);
	hIP = GetDlgItem(hDlg, IDC_IP);
	hTCP = GetDlgItem(hDlg, IDC_TCP);
	hUDP = GetDlgItem(hDlg, IDC_UPD);
	hPort = GetDlgItem(hDlg, IDC_PORTNUMBER);
	hInputType = GetDlgItem(hDlg, IDC_InputType);
	hHost = GetDlgItem(hDlg, IDC_Host);
	hPSize = GetDlgItem(hDlg, IDC_PSIZE);
	hPNum = GetDlgItem(hDlg, IDC_PNUM);
	hStatus = GetDlgItem(hDlg, IDC_STATUS);
	hFilename = GetDlgItem(hDlg, IDC_FILENAME);
	hFileIO = GetDlgItem(hDlg, IDC_FILE);
	hConnect = GetDlgItem(hDlg, IDC_SEND);
	SendMessage(hInputType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"Host Name");
	SendMessage(hInputType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"IP Address");

	SetWindowText(hPort, "7000");
	SetWindowText(hPSize, "2000");
	SetWindowText(hPNum, "1");
	SetWindowText(hIP, "192.168.1.67");

	InitFileOpener();
}

void EnableGUI(BOOL isEnable)
{
	SetWindowText(hConnect, (isEnable) ? "Send" : "Connect");
	CurrentMode = (isEnable) ? CLIENT_MODE : SERVER_MODE;
	EnableWindow(hIP, isEnable);
	EnableWindow(hPSize, isEnable);
	EnableWindow(hInputType, isEnable);
	EnableWindow(hHost, isEnable);
	EnableWindow(hPNum, isEnable);
	EnableWindow(hFilename, isEnable);
	EnableWindow(hFileIO, isEnable);
}

void FileManager()
{
	if (GetOpenFileName(&ofn) == TRUE)
		fileReadWriteThread = CreateThread(NULL, 0, createFileReader, NULL, 0, &fileReadWriteID);
	loadFileToView(&readFrom, ofn.lpstrFile);
}

void loadFileToView(std::string * readFrom, LPSTR file) {
	std::string tmp;
	std::ifstream inputF(file);
	
	while (getline(inputF, tmp))
	{
		//store data here	
	}
	inputF.close();
}

void InitFileOpener() {
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_HIDEREADONLY;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Text Files\0*.txt\0";
	ofn.lpstrDefExt = "txt";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
}

DWORD WINAPI createFileReader(LPVOID lpParam) {
	hf = CreateFile(ofn.lpstrFile,
		GENERIC_READ,
		0,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)hDlg);
	return 0;
}

void AppendToStatus(HWND handle, char * buffer)
{
	int index = GetWindowTextLength(handle);
	SetFocus(handle); // set focus
	SendMessageA(handle, EM_SETSEL, (WPARAM)index, (LPARAM)index); // set selection - end of text
	SendMessageA(handle, EM_REPLACESEL, 0, (LPARAM)buffer); // append!
}