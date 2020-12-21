#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <CommCtrl.h>
#include <thread>
#include <vector>
#include <string>
#include <Richedit.h>
#include <wingdi.h>
#include "resource1.h"
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
auto slowT = std::chrono::milliseconds(50); //standby
int processloopratio = 10; //process windows every ratio*t milliseconds (10*50=500)
int processloop = 0;
BOOL exists = false;
BOOL diaconfirmed = false;
auto bkgcolor = RGB(240, 240, 240);

struct Win {
	HWND hwnd;
	std::string OGname;
	std::string NEWname;
};
HWND activewin;
std::vector<Win> wins;
char buffer[255];
std::vector<int> closed;
int wini = 0;

void processWindows() {
	wini = 0;
	for (Win& w : wins) {
		if (IsWindow(w.hwnd)) {
			GetWindowText(w.hwnd, buffer, 255);
			if (strcmp(buffer, w.NEWname.c_str()) != 0) {
				w.OGname = buffer;
				SetWindowTextA(w.hwnd, w.NEWname.c_str());
			}
		}
		else {
			closed.emplace_back(wini);
		}
		++wini;
	}
	for (auto it = closed.rbegin(); it != closed.rend(); ++it)
	{
		wins.erase(wins.begin() + *it);
	}
	closed.clear();
}

BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		GetWindowText(activewin, buffer, 255);
		SetWindowText(GetDlgItem(hwndDlg, IDC_RICHEDIT21), buffer);
		SendMessageA(GetDlgItem(hwndDlg, IDC_RICHEDIT21), EM_SETSEL, 0, -1);
		SetFocus(GetDlgItem(hwndDlg, IDC_RICHEDIT21));
		SendMessageA(GetDlgItem(hwndDlg, IDC_RICHEDIT21), EM_SETBKGNDCOLOR, 0, RGB(240, 240, 240));
		diaconfirmed = false;
		return false;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			diaconfirmed = true;
			GetWindowText(GetDlgItem(hwndDlg, IDC_RICHEDIT21), buffer, 255);
			EndDialog(hwndDlg, 1);
			return true;
		case IDRESET:
			diaconfirmed = true;
			EndDialog(hwndDlg, 2);
			return true;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return true;
		}
		return false;
	case WM_NCACTIVATE:
		if (wParam == 0 && !diaconfirmed) {
			EndDialog(hwndDlg, 0);
		}
		return true;
	default:
		return false;
	}
}

int main() {
	LoadLibrary("RichEd20.dll");
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_F7);
	MSG msg = { 0 };
	while (true) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			if (msg.message == WM_HOTKEY)
			{
				activewin = GetForegroundWindow();
				switch (DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), activewin, DlgProc)) {
				case 1: //ok
					exists = false;
					for (Win& w : wins) {
						if (w.hwnd == activewin) {
							exists = true;
							w.NEWname = buffer;
							SetWindowTextA(w.hwnd, buffer);
							break;
						}
					}
					if (!exists) {
						Win w;
						w.hwnd = activewin;
						w.NEWname = buffer;
						wins.emplace_back(w);
					}
					break;
				case 2: //reset
					wini = 0;
					for (Win& w : wins) {
						if (w.hwnd == activewin) {
							w.NEWname = w.OGname;
							SetWindowTextA(w.hwnd, w.OGname.c_str());
							wins.erase(wins.begin() + wini);
							break;
						}
						++wini;
					}
					break;
				}
			}
		}

		if (++processloop >= processloopratio) {
			processloop = 0;
			processWindows();
		}
		std::this_thread::sleep_for(slowT);
	}
}