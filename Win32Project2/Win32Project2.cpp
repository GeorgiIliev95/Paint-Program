// Win32Project2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project2.h"
#include <string>
#include <Windows.h>
#include <Commdlg.h>
#include <tchar.h>
#include <atlimage.h>
#include <fstream>
#include <istream>
#include "resource.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int top, bottom, left, right;
bool flagRectangle=false;
HWND hwndToolBox=NULL;
HWND hwndBoxStyles = NULL;//combo box handler
HWND hwndEraser = NULL;//handler for eraser radio button
HWND hwndEllipse = NULL;//handler for ellipse
HWND hwndRect = NULL;
HWND hwndBrush = NULL;
HWND hwndLine = NULL;
bool line = false;
bool brush = false;
bool ellipse=false;
bool rect = false;
bool lineSolid = false;
bool lineDash = false;
bool lineDot = false;
bool lineDashDot = false;
bool eraser = false;
bool lbDown = false;
int r=0, g=0, b=0;
HWND hwndDisplay = NULL;
HCURSOR hCur;
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM				MyRegisterClassCur(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Toolbox(HWND, UINT, WPARAM, LPARAM);

void CaptureScreen(HWND window, const char* filename)
{
	// get screen rectangle
	RECT windowRect = {0};
	
	
	// bitmap dimensions
	int bitmap_dx = 1190;
	int bitmap_dy =700;

	// create file
	std::ofstream file(filename, std::ios::binary);
	if (!file) return;

	// save bitmap file headers
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	fileHeader.bfType = 0x4d42;
	fileHeader.bfSize = 0;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	infoHeader.biSize = sizeof(infoHeader);
	infoHeader.biWidth = bitmap_dx;
	infoHeader.biHeight = bitmap_dy;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = BI_RGB;
	infoHeader.biSizeImage = 0;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;

	file.write((char*)&fileHeader, sizeof(fileHeader));
	file.write((char*)&infoHeader, sizeof(infoHeader));

	// dibsection information
	BITMAPINFO info;
	info.bmiHeader = infoHeader;

	// ------------------
	// THE IMPORTANT CODE
	// ------------------
	// create a dibsection and blit the window contents to the bitmap
	HDC winDC = GetWindowDC(window);
	HDC memDC = CreateCompatibleDC(winDC);
	BYTE* memory = 0;
	HBITMAP bitmap = CreateDIBSection(winDC, &info, DIB_RGB_COLORS, (void**)&memory, 0, 0);
	SelectObject(memDC, bitmap);
	BitBlt(memDC, 0,0, bitmap_dx, bitmap_dy, winDC, 10,50, SRCCOPY);
	DeleteDC(memDC);
	ReleaseDC(window, winDC);

	// save dibsection data
	int bytes = (((24 * bitmap_dx + 31) & (~31)) / 8)*bitmap_dy;
	file.write((char*)memory, bytes);

	
	DeleteObject(bitmap);
}


bool LoadBMPIntoDC(HDC hDC,LPCTSTR bmpFile,int width, int height)
{
	if ((NULL==hDC)||(NULL==bmpFile))
	{
		return false;
	}
	HANDLE hBmp = LoadImage(NULL, bmpFile, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE);
	if (NULL == hBmp)
		return false;
	HDC dcmem = CreateCompatibleDC(NULL);
	if (NULL == SelectObject(dcmem, hBmp))
	{	// failed to load bitmap into device context
		DeleteDC(dcmem);
		return false;
	}
	BITMAP bm;
	GetObject(hBmp, sizeof(bm), &bm);
	//if (BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, dcmem,
	//	0, 0, SRCCOPY) == 0)
	//{	// failed the blit
	//	DeleteDC(dcmem);
	//	return false;
	//}
	if (StretchBlt(hDC,left,top,width,height,dcmem,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY)==0)
	{
		// failed the blit
			DeleteDC(dcmem);
			return false;
	}
	DeleteDC(dcmem);  // clear up the memory dc	
	return true;
}
VOID Eraser(HDC hdc, int x1, int y1, int x2, int y2)//left,top,bottom,right;
{
	
	HPEN hLinePen;
	COLORREF qLineColor;
	qLineColor = RGB(r, g, b);
	hLinePen = CreatePen(PS_NULL, 1, qLineColor);
	SelectObject(hdc, hLinePen);
	Rectangle(hdc, x1, y1, x2, y2);

	SelectObject(hdc, hLinePen);
	DeleteObject(hLinePen);
}
VOID BRUSH(HDC hdc, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	HPEN hLinePen = CreatePen(PS_NULL, 1, RGB(r, g, b));
	SelectObject(hdc, hLinePen);
	HBRUSH hBrush = CreateSolidBrush(RGB(r, g, b));
	SelectObject(hdc, hBrush);

	Ellipse(hdc, x1, y1, x2, y2);
	

	
	SelectObject(hdc, hBrush);
	DeleteObject(hBrush);
}
VOID OnPaintEllipse(HDC hdc, int x1, int y1, int x2, int y2, int Style, int r, int g, int b)
{

	HPEN hPenOld;
	HPEN hLinePen;
	COLORREF qLineColor;
	qLineColor = RGB(r, g, b);
	hLinePen = CreatePen(Style, 1, qLineColor);
	hPenOld = (HPEN)SelectObject(hdc, hLinePen);

	Ellipse(hdc, x1, y1, x2, y2);

	SelectObject(hdc, hPenOld);
	DeleteObject(hLinePen);
}
VOID OnPaintLineRect(HDC hdc, int x1, int y1, int x2, int y2, int Style, int r, int g, int b)
{

	HPEN hPenOld;
	HPEN hLinePen;
	COLORREF qLineColor;
	qLineColor = RGB(r, g, b);
	hLinePen = CreatePen(Style, 1, qLineColor);
	hPenOld = (HPEN)SelectObject(hdc, hLinePen);

	Rectangle(hdc, x1, y1, x2, y2);

	SelectObject(hdc, hPenOld);
	DeleteObject(hLinePen);
}

VOID OnPaintLine(HDC hdc,int x1,int y1,int x2,int y2,int Style,int r,int g,int b)
{
	
	HPEN hPenOld;
	HPEN hLinePen;
	COLORREF qLineColor;
	qLineColor = RGB(r,g,b);
	hLinePen = CreatePen(Style, 1, qLineColor);
	hPenOld = (HPEN)SelectObject(hdc, hLinePen);

	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);

	SelectObject(hdc, hPenOld);
	DeleteObject(hLinePen);
}
VOID OnPaintBrushEllipse(HDC hdc, int x1, int y1, int x2, int y2, int r, int g, int b)
{

	HBRUSH hBrush = CreateSolidBrush(RGB(r, g, b));

	SelectObject(hdc, hBrush);
	Ellipse(hdc, x1, y1, x2, y2);

	SelectObject(hdc, hBrush);
	DeleteObject(hBrush);
}
VOID OnPaintBrush(HDC hdc, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	
	HBRUSH hBrush=CreateSolidBrush(RGB(r,g,b));
	
	SelectObject(hdc, hBrush);
	Rectangle(hdc, x1, y1, x2, y2);

	SelectObject(hdc, hBrush);
	DeleteObject(hBrush);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32PROJECT2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	BOOL bRet;

	
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN32PROJECT2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


	
    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   hwndDisplay = CreateWindow(0, 0, WS_CHILD | WS_VISIBLE, 0, 0, 1200, 350, hWnd, NULL, hInstance, NULL);
   hwndLine = CreateWindow(
	  (LPCSTR)"BUTTON",  // Predefined class; Unicode assumed 
	  (LPCSTR)"Line",      // Button text 
	   WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,  // Styles 
	   1200,         // x position 
	   400,         // y position 
	   50,        // Button width
	   50,        // Button height
	   hWnd,     // Parent window
	   (HMENU)RB_LINE,
	   hInstance,
	   NULL);      // Pointer not needed.
   hwndBrush = CreateWindow(
	  (LPCSTR)"BUTTON",  // Predefined class; Unicode assumed 
	  (LPCSTR)"Brush",      // Button text 
	  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles 
	  1200,         // x position 
	  450,         // y position 
	  90,        // Button width
	  20,        // Button height
	  hWnd,     // Parent window
	  (HMENU)RB_BRUSH,       // No menu.
	  hInstance,
	  NULL);      // Pointer not needed.
   hwndRect = CreateWindow(
	  (LPCSTR)"BUTTON",  // Predefined class; Unicode assumed 
	  (LPCSTR)"Rectangle",      // Button text 
	   WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,  // Styles 
	   1200,         // x position 
	   500,         // y position 
	   90,        // Button width
	   20,        // Button height
	   hWnd,     // Parent window
	   (HMENU)RB_RECT,       // No menu.
	   hInstance,
	   NULL);      // Pointer not needed.
	hwndEllipse = CreateWindow(
	  (LPCSTR)"BUTTON",  // Predefined class; Unicode assumed 
	  (LPCSTR)"Ellipse",      // Button text 
	  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON ,  // Styles 
	  1200,         // x position 
	  520,         // y position 
	  90,        // Button width
	  20,        // Button height
	  hWnd,     // Parent window
	  (HMENU)RB_ELLIPSE,       // No menu.
	  hInstance,
	  NULL);
	hwndEraser = CreateWindow(
	  (LPCSTR)"BUTTON",  // Predefined class; Unicode assumed 
	  (LPCSTR)"Eraser",      // Button text 
	  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles 
	  1200,         // x position 
	  570,         // y position 
	  90,        // Button width
	  20,        // Button height
	  hWnd,     // Parent window
	  (HMENU)RB_ERASER,       // No menu.
	  hInstance,
	  NULL);
   hwndBoxStyles = CreateWindow(
	   (LPCSTR)"COMBOBOX",
	   (LPCSTR)"Styles:",
	  CBS_DROPDOWN | WS_CHILD| WS_VISIBLE,
	  1260,
	  410,
	  110,
	  100,
	  hWnd,
	  (HMENU)CB_STYLES,
	  hInstance,
	  NULL );
  SendMessage(hwndBoxStyles, CB_ADDSTRING, 0, (LPARAM)_T("Solid Pen"));
  SendMessage(hwndBoxStyles, CB_ADDSTRING, 1, (LPARAM)_T("Dash Pen"));
  SendMessage(hwndBoxStyles, CB_ADDSTRING, 2, (LPARAM)_T("Dot Pen"));
  SendMessage(hwndBoxStyles, CB_ADDSTRING, 3, (LPARAM)_T("Dash Dot"));
  
  SendMessage(hwndBoxStyles, CB_SETCURSEL, 0, 0);

  HWND hwnColor = CreateWindow(
	  (LPCSTR)"BUTTON",  // Predefined class; Unicode assumed 
	  (LPCSTR)"Pick Color",      // Button text 
	  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
	  1200,         // x position 
	  600,         // y position 
	  90,        // Button width
	  20,        // Button height
	  hWnd,     // Parent window
	  (HMENU)BT_COLOR,       
	  hInstance,
	  NULL);      // Pointer not needed.

  //MyRegisterClassCur(hInstance);
	
   if (!hWnd)
   {
      return FALSE;
   }

  
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    switch (message)
    {
	case WM_CREATE:
	{
		
		
		break;
	}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
			switch (wmId)
			{

			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			case WM_SETCURSOR:
			{
			
				break;
			}
			case ID_FILE_SAVE:
			{
				
				OPENFILENAME ofn;
				char szFile[100];
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				GetSaveFileName(&ofn);

				MessageBox(NULL, ofn.lpstrFile, "File Name", MB_OK);
				HWND  hwndSave = CreateWindowEx(
					0,                   /* Extended possibilites for variation */
					NULL,         /* Classname */
					NULL,       /* Title Text */
					NULL, /* default window */
					0,       /* Windows decides the position */
					0,       /* where the window ends up on the screen */
					1200,                 /* The programs width */
					400,                 /* and height in pixels */
					HWND_DESKTOP,        /* The window is a child-window to desktop */
					NULL,                /* No menu */
					 NULL,       /* Program Instance handler */
					NULL                 /* No Window Creation data */
					);
				
				CaptureScreen(hWnd, ofn.lpstrFile);

				break;
			}
			case ID_FILE_OPEN:
			{
				if (!rect)
				{
					MessageBox(NULL, "You need to draw a rectangle first!", "Warning", MB_OK );
					break;
				}
				OPENFILENAME ofn;
			char szFile[100];
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			GetOpenFileName(&ofn);

			// Now simpley display the file name 
			//MessageBox(NULL, ofn.lpstrFile, "File Name", MB_OK);
			HDC dc = GetDC(hWnd);
			LoadBMPIntoDC(dc,ofn.lpstrFile,(bottom-left), (right - top));
			ReleaseDC(hWnd, dc);

				break;
			}
			case RB_LINE:
			{
				line = true;
				brush = false;
				eraser = false;
				SendMessage(hwndEraser, BM_SETCHECK, BST_UNCHECKED, 0);
				WNDCLASS wc;

				wc.style = NULL;                        // class style(s) 
				wc.lpfnWndProc = WndProc; // window procedure 
				wc.cbClsExtra = 0;           // no per-class extra data 
				wc.cbWndExtra = 0;           // no per-window extra data 
				wc.hInstance = hInst;        // application that owns the class 
				wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
				wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_PEN)); // class cursor 
				wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
				wc.lpszMenuName = "GenericMenu";               // class menu 
				wc.lpszClassName = "GenericWClass";              // class name 
				SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				break;
			}
			case RB_RECT:
			{
				rect = true;
				ellipse = false;
				eraser = false;
				SendMessage(hwndEraser, BM_SETCHECK, BST_UNCHECKED, 0);
				if (brush)
				{
					WNDCLASS wc;

					wc.style = NULL;                        // class style(s) 
					wc.lpfnWndProc = WndProc; // window procedure 
					wc.cbClsExtra = 0;           // no per-class extra data 
					wc.cbWndExtra = 0;           // no per-window extra data 
					wc.hInstance = hInst;        // application that owns the class 
					wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
					wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_BRUSH)); // class cursor 
					wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
					wc.lpszMenuName = "GenericMenu";               // class menu 
					wc.lpszClassName = "GenericWClass";              // class name 
					SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				}
				else
				{
					WNDCLASS wc;

					wc.style = NULL;                        // class style(s) 
					wc.lpfnWndProc = WndProc; // window procedure 
					wc.cbClsExtra = 0;           // no per-class extra data 
					wc.cbWndExtra = 0;           // no per-window extra data 
					wc.hInstance = hInst;        // application that owns the class 
					wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
					wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_PEN)); // class cursor 
					wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
					wc.lpszMenuName = "GenericMenu";               // class menu 
					wc.lpszClassName = "GenericWClass";              // class name 
					SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				}
				break;
			}
			case RB_ELLIPSE:
			{
				ellipse = true;
				rect = false;
				eraser = false;
				SendMessage(hwndEraser, BM_SETCHECK, BST_UNCHECKED, 0);
				if (brush)
				{
					WNDCLASS wc;

					wc.style = NULL;                        // class style(s) 
					wc.lpfnWndProc = WndProc; // window procedure 
					wc.cbClsExtra = 0;           // no per-class extra data 
					wc.cbWndExtra = 0;           // no per-window extra data 
					wc.hInstance = hInst;        // application that owns the class 
					wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
					wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_BRUSH)); // class cursor 
					wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
					wc.lpszMenuName = "GenericMenu";               // class menu 
					wc.lpszClassName = "GenericWClass";              // class name 
					SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				}
				else
				{
					WNDCLASS wc;

					wc.style = NULL;                        // class style(s) 
					wc.lpfnWndProc = WndProc; // window procedure 
					wc.cbClsExtra = 0;           // no per-class extra data 
					wc.cbWndExtra = 0;           // no per-window extra data 
					wc.hInstance = hInst;        // application that owns the class 
					wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
					wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_PEN)); // class cursor 
					wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
					wc.lpszMenuName = "GenericMenu";               // class menu 
					wc.lpszClassName = "GenericWClass";              // class name 
					SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				}
				break;
			}
			case RB_BRUSH:
			{
				brush = true;
				line = false;
				eraser = false;
				SendMessage(hwndEraser, BM_SETCHECK, BST_UNCHECKED, 0);
				
				WNDCLASS wc;

				wc.style = NULL;                        // class style(s) 
				wc.lpfnWndProc = WndProc; // window procedure 
				wc.cbClsExtra = 0;           // no per-class extra data 
				wc.cbWndExtra = 0;           // no per-window extra data 
				wc.hInstance = hInst;        // application that owns the class 
				wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
				wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_BRUSH)); // class cursor 
				wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
				wc.lpszMenuName = "GenericMenu";               // class menu 
				wc.lpszClassName = "GenericWClass";              // class name 
				SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				break;

			}
			case RB_ERASER:
			{
				eraser = true;	
				line = false;
				brush = false;
				ellipse = false;
				rect = false;
				
				SendMessage(hwndLine, BM_SETCHECK, BST_UNCHECKED, 0);
				SendMessage(hwndBrush, BM_SETCHECK, BST_UNCHECKED, 0);
				SendMessage(hwndRect, BM_SETCHECK, BST_UNCHECKED, 0);
				SendMessage(hwndEllipse, BM_SETCHECK, BST_UNCHECKED, 0);
				
				
				
				WNDCLASS wc;
				
				wc.style = NULL;                        // class style(s) 
				wc.lpfnWndProc = WndProc; // window procedure 
				wc.cbClsExtra = 0;           // no per-class extra data 
				wc.cbWndExtra = 0;           // no per-window extra data 
				wc.hInstance = hInst;        // application that owns the class 
				wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // class icon 
				wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_ERASER)); // class cursor 
				wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // class background 
				wc.lpszMenuName = "GenericMenu";               // class menu 
				wc.lpszClassName = "GenericWClass";              // class name 
				SetClassLong(hWnd, GCL_HCURSOR, (LONG)wc.hCursor);
				
				break;
			}
			case BT_COLOR:
			{
				CHOOSECOLOR color;//the struct for the dialog
				COLORREF ccref[16];//custom colors
				COLORREF selcolor = 0x000000;//the default selected color

				memset(&color, 0, sizeof(color));
				color.lStructSize = sizeof(CHOOSECOLOR);
				color.hwndOwner = hWnd;
				color.lpCustColors = ccref;
				color.rgbResult = selcolor;
				color.Flags = CC_RGBINIT;

				if (ChooseColor(&color))
				{
					selcolor = color.rgbResult;
					//redraw with the new color
				}
				r =	GetRValue(color.rgbResult);
				g = GetGValue(color.rgbResult);
				b = GetBValue(color.rgbResult);
				break;
			}
			case CB_STYLES:
			{
				if (HIWORD(wParam)== CBN_SELCHANGE)
				{
					int idx;

					idx = SendMessage(hwndBoxStyles, CB_GETCURSEL, 0, 0);
					//TCHAR buf[100];
//					_stprintf(buf, _T("%d"), idx);
					//MessageBox(hWnd, buf, "Testing", MB_OK);
					
					switch (idx)
					{
						case 0: 
						{
							lineSolid = true;
							lineDash = false;
							lineDot = false;
							lineDashDot = false;
						}
							break;
						case 1:
						{
							lineSolid = false;
							lineDash = true;
							lineDot = false;
							lineDashDot = false;
						}
							break;
						case 2:
						{
							lineSolid = false;
							lineDash = false;
							lineDot = true;
							lineDashDot = false;
						}
							break;
						case 3:
						{
							lineSolid = false;
							lineDash = false;
							lineDot = false;
							lineDashDot = true;
						}
							break;
					default:
						break;
					}
				}

				break;
			}
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
        }
        break;
	case WM_LBUTTONDOWN:
	{
		
		left = LOWORD(lParam);
		top = HIWORD(lParam);
		lbDown = true;
		if (brush && !rect &&!ellipse)
		{
		//	MessageBox(hWnd, "You need to select Rectangle or Ellipse when using a brush!", "Warning", MB_OK);
		}
		//RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);
		break;
	}
   case WM_LBUTTONUP:
	{
		bottom = LOWORD(lParam);
		right = HIWORD(lParam);
		lbDown = false;
		RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);
		break;
	}
   case WM_MOUSEMOVE:
   {
	  // RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);
	   if (eraser&&lbDown)
	   {
		   left = LOWORD(lParam);
		   top = HIWORD(lParam);
		   RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);
	   }
	   if (brush&&lbDown&&!rect&&!ellipse)
	   {
		   left = LOWORD(lParam);
		   top = HIWORD(lParam);
		   RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);
	   }
	  
   }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			MoveToEx(hdc, 1195, 0, NULL);
			LineTo(hdc, 1195, 701);
			MoveToEx(hdc, 0, 701, NULL);
			LineTo(hdc, 1195, 701);
			MoveToEx(hdc, 0, 0, NULL);
			
			if (line && lineSolid &&!ellipse && !rect ||(line&&!lineSolid&&!lineDash&&!lineDot&&!lineDashDot&&!ellipse&&!rect) )
			{
				OnPaintLine(hdc, left, top, bottom, right,0,r,g,b);
				
			}
			if (line && lineDash && !ellipse && !rect)
			{
				OnPaintLine(hdc, left, top, bottom, right, 1,r,g,b);
			}
			if (line && lineDot && !ellipse && !rect)
			{
				OnPaintLine(hdc, left, top, bottom, right, 2, r, g, b);
			}
			if (line&&lineDashDot && !ellipse && !rect)
			{
				OnPaintLine(hdc, left, top, bottom, right, 3, r, g, b);
			}
			if (brush&&!rect&&!ellipse)
			{
				BRUSH(hdc, left, top, left + 10, top + 10, r, g, b); 
			}
			
			if (rect && brush)
			{
				OnPaintBrush(hdc, left, top, bottom, right, r, g, b);
			}
			if (rect && line&& lineSolid || (rect && line&&!lineSolid&&!lineDash&&!lineDot&&!lineDashDot))
			{
				OnPaintLineRect(hdc, left, top, bottom, right, 0, r, g, b);
			}
			if (rect && line &&lineDash)
			{
				OnPaintLineRect(hdc, left, top, bottom, right, 1, r, g, b);
			}
			if (rect && line && lineDot)
			{
				OnPaintLineRect(hdc, left, top, bottom, right, 2, r, g, b);
			}
			if (rect && line && lineDashDot)
			{
				OnPaintLineRect(hdc, left, top, bottom, right, 3, r, g, b);
			}



			if (ellipse && brush)
			{
				OnPaintBrushEllipse(hdc, left, top, bottom, right, r, g, b);
			}
			if (ellipse && line&& lineSolid || (ellipse && line&&!lineSolid&&!lineDash&&!lineDot&&!lineDashDot))
			{
				OnPaintEllipse(hdc, left, top, bottom, right, 0, r, g, b);
			}
			if (ellipse && line &&lineDash)
			{
				OnPaintEllipse(hdc, left, top, bottom, right, 1, r, g, b);
			}
			if (ellipse && line && lineDot)
			{
				OnPaintEllipse(hdc, left, top, bottom, right, 2, r, g, b);
			}
			if (ellipse && line && lineDashDot)
			{
				OnPaintEllipse(hdc, left, top, bottom, right, 3, r, g, b);
			}

			if (eraser)
			{
				
				Eraser(hdc, left, top, left+25, top+25);
			}
			MoveToEx(hdc, 1195, 0, NULL);
			LineTo(hdc, 1195, 701);
			MoveToEx(hdc, 0, 701, NULL);
			LineTo(hdc, 1195, 701);
			MoveToEx(hdc, 0, 0, NULL);
            EndPaint(hWnd, &ps);
        }
        break;
		
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
