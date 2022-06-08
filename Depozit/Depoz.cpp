#include "Depoz.h"

Depoz* Depoz::ptr = NULL;
CRITICAL_SECTION cs;
int procent;
int sum = 1000;
HWND hEdit[6], hButton[2];
TCHAR Text[60], Answer1[20], Answer2[20], Answer3[20];

Depoz::Depoz(void)
{
	ptr = this;
}

Depoz::~Depoz(void)
{
	ReleaseMutex(hMutex);
}

void Depoz::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

void MessageAboutError(DWORD dwError)
{
	LPVOID lpMsgBuf = NULL;
	TCHAR szBuf[300];

	BOOL fOK = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if (lpMsgBuf != NULL)
	{
		wsprintf(szBuf, TEXT("Ошибка %d: %s"), dwError, lpMsgBuf);
		MessageBox(0, szBuf, TEXT("Сообщение об ошибке"), MB_OK | MB_ICONSTOP);
		LocalFree(lpMsgBuf);
	}
}

DWORD WINAPI WriteToEdit(LPVOID lp)
{
	HWND Edit = (HWND)lp;
	HANDLE h = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, TEXT("{41B4DBD4-F00A-4999-BFA9-1A20D12591B1}"));
	if (WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0)
	{
		EnterCriticalSection(&cs);
		GetWindowText(hEdit[0], Answer1, 20);
		GetWindowText(hEdit[1], Answer2, 20);
		GetWindowText(hEdit[2], Answer3, 20);
		wsprintf(Text, TEXT("%s \n %s \n %s"), Answer1, Answer2, Answer3);
		SetWindowText(Edit, Text);
		LeaveCriticalSection(&cs);
		MessageBox(0, TEXT("Поток записал фио в поле"), TEXT("Критическая секция"), MB_OK); 
		ReleaseSemaphore(h, 1, NULL);
	}
	CloseHandle(h);
	return 1;

	
}

DWORD WINAPI Write(LPVOID lp)
{ 
	HWND hEdit = (HWND)lp;
	HANDLE h = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, TEXT("{41B4DBD4-F00A-4999-BFA9-1A20D12591B1}"));
	if (WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0)
	{
		EnterCriticalSection(&cs);
		FILE* fp;
		fp = fopen("array.txt", "r+");
		fwrite(Answer1, 20, 1, fp);
		fwrite(Answer2, 20, 1, fp);
		fwrite(Answer3, 20, 1, fp);
		fclose(fp);
		LeaveCriticalSection(&cs);
		MessageBox(0, TEXT("Поток записал информацию в файл"), TEXT("Критическая секция"), MB_OK); 
		ReleaseSemaphore(h, 1, NULL);
	}
	CloseHandle(h);
	return 1; 
}

DWORD WINAPI Read(LPVOID lp)
{
	HWND hEdit = (HWND)lp;
	HANDLE h = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, TEXT("{41B4DBD4-F00A-4999-BFA9-1A20D12591B1}"));
	if (WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0)
	{
		EnterCriticalSection(&cs);
		ifstream in(TEXT("array.txt"));
		if (!in.is_open())
		{
			MessageAboutError(GetLastError());
			return 1;
		}
		char B[60];
		for (int i = 0; i < 60; i++) in >> B[i];
		in.close();
		LeaveCriticalSection(&cs);
		MessageBox(0, TEXT("Поток прочитал информацию из файла"), TEXT("Критическая секция"), MB_OK);
		TCHAR str[30];
		wsprintf(str, TEXT("%s"), B);
		SetWindowText(hEdit, str); 
		ReleaseSemaphore(h, 1, NULL);
	}
	CloseHandle(h);
	return 1;



	
}

DWORD WINAPI CalulateProcent(LPVOID lp)
{  
	HWND hEdit = (HWND)lp;
	HANDLE h = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, TEXT("{41B4DBD4-F00A-4999-BFA9-1A20D12591B1}"));
	if (WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0)
	{
		EnterCriticalSection(&cs);
		procent = _wtoi(Answer3);
		sum = _wtoi(Answer2);
		for (int i = 0; i < 12; i++) sum += (sum * procent) / 100;
		wsprintf(Text, TEXT("%d"), sum);
		SetWindowText(hEdit, Text);

		LeaveCriticalSection(&cs);
		MessageBox(0, TEXT("Поток записал сумму в поле"), TEXT("Критическая секция"), MB_OK); 
		ReleaseSemaphore(h, 1, NULL);
	}
	CloseHandle(h);
	return 1;  
}

BOOL Depoz::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	TCHAR GUID[] = TEXT("text");
	hMutex = CreateMutex(NULL, FALSE, GUID);
	DWORD dwAnswer = WaitForSingleObject(hMutex, 0);
	InitializeCriticalSection(&cs);
	if (dwAnswer == WAIT_TIMEOUT)
	{
		MessageBox(hwnd, TEXT("Нельзя запускать более одной копии приложения!!!"), TEXT("Мьютекс"), MB_OK | MB_ICONINFORMATION);
		EndDialog(hwnd, 0);
	}
	hButton[0] = GetDlgItem(hwnd, IDC_BUTTON1);
	hButton[1] = GetDlgItem(hwnd, IDC_BUTTON2);

	for (int i = 0; i < 6; i++)
	{
		hEdit[i] = GetDlgItem(hwnd, IDC_EDIT1 + i);
	}

	return TRUE;
}

void Depoz::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BUTTON1)
	{ 
		HANDLE hSemaphore = CreateSemaphore(NULL, 3, 3, TEXT("{41B4DBD4-F00A-4999-BFA9-1A20D12591B1}"));
		HANDLE hThread = CreateThread(NULL, 0, WriteToEdit, hEdit[4], 0, NULL);
		CloseHandle(hThread);
		hThread = CreateThread(NULL, 0, CalulateProcent, hEdit[5], 0, NULL);
		CloseHandle(hThread);
		hThread = CreateThread(NULL, 0, Write, 0, 0, NULL);
		CloseHandle(hThread); 
		hThread = CreateThread(NULL, 0, Read, hEdit[4], 0, NULL);
		CloseHandle(hThread);
	}
	
}

BOOL CALLBACK Depoz::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}