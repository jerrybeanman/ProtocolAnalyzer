#include "Global.h"

/* Function prototype */
INT_PTR CALLBACK WndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
MSG Msg;

DWORD CurrentMode;
DWORD CurrentProtocol;
DWORD CurrentSelectedInputType;


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow) {



	/* Sets the dialog box defined in Reource file as the main window */
	hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, WndProc, 0);
	ShowWindow(hDlg, nCmdShow);

	FindDialogItems();

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

INT_PTR CALLBACK WndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		/* Check which serivice is being check */
		if (IsDlgButtonChecked(hDlg, IDC_SERVER) == BST_CHECKED)
			EnableGUI(FALSE);
		else if (IsDlgButtonChecked(hDlg, IDC_CLIENT) == BST_CHECKED)
			EnableGUI(TRUE);
		
		/* check which type of service */
		if (CurrentMode == CLIENT_MODE)
			/* Go into client mode */
			ClientManager(wParam);
		else
		if(CurrentMode == SERVER_MODE)
			/* Go into server mode */
			ServerManager(wParam);

		/* Show input field based on the current input type user selected */
		if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_InputType)
		{
			/* Gets the currently selected item and assign it to item_selected buffer */
			switch ((WPARAM)SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0))
			{
			case 0:
				CurrentSelectedInputType = HOST_INPUT;
				ShowWindow(hIP, SW_HIDE);
				ShowWindow(hHost, SW_SHOW);
				break;
			case 1:
				CurrentSelectedInputType = IP_INPUT;
				ShowWindow(hHost, SW_HIDE);
				ShowWindow(hIP, SW_SHOW);
				break;
			}
		}

		/* if send file is being pressed */
		if (LOWORD(wParam) == IDC_FILE && HIWORD(wParam) == BN_CLICKED)
		{
			FileManager();
		}
		break;
	}
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hDlg, uMsg, wParam, lParam);;
	}
}