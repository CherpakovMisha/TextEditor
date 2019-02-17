#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "menu.h"
#include "text_structs.h"
#include "text_functions.h"



/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
WCHAR szClassName[ ] = L"TextEdit";
#define WND_CLASS_NAME "1"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, CHAR *CmdLine, INT ShowCmd)
{
    WNDCLASS wc;
    HWND hWnd;
    MSG msg;

    wc.style = CS_VREDRAW | CS_HREDRAW;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProcedure;
    wc.lpszClassName = WND_CLASS_NAME;
    wc.lpszMenuName = MAKEINTRESOURCE(MYMENU);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RegisterClass(&wc);

    hWnd = CreateWindowA("1", "1", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, 0, 0, 500, 300, NULL, NULL, hInstance, CmdLine);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0) != 0 )
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    return msg.wParam;
}



void CreatingWindow(const HWND hwnd, const LPARAM *lParam, WINDOW_INFO *windowInfo)
{
    static HBITMAP hBm;
    HDC hdc = GetDC(hwnd);
    HFONT font;

    windowInfo->hMemDC = CreateCompatibleDC(hdc);
    hBm = CreateCompatibleBitmap(hdc, maxWidth, maxHeight);
    SelectObject(windowInfo->hMemDC, hBm);

    ReleaseDC(hwnd, hdc);

    font = CreateFont(18,          // height 18pt
        8,                         // symbol width
        0,                         // angle 0.1
        0,                         // angle 0.01
        FW_BOLD,                   // thickness: 0 to 1000
        0,                         // italic
        0,                         // underline
        0,                         // strikeout
        DEFAULT_CHARSET,           // character set identifier
        OUT_DEVICE_PRECIS,         // output precision
        CLIP_DEFAULT_PRECIS,       // clipping precision
        DEFAULT_QUALITY,           // output quality
        DEFAULT_PITCH,             // pitch and family
        "Courier");
    SelectObject(windowInfo->hMemDC, font);

    return;
}


void Clean_Window(const HDC hMemDC)
{
    static HBRUSH hBrush;
    static HPEN hPen;

    hPen = CreatePen(0, 1, RGB(255, 255, 255));
    hBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hMemDC, hPen);
    SelectObject(hMemDC, hBrush);
    Rectangle(hMemDC, 0, 0, maxWidth, maxHeight);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static WINDOW_INFO windowInfo;
    static TEXT_DATA text;

    //windowInfo.modeChecked = IDM_DEFAULT;
    switch (message)
    {
        case WM_CREATE:{
            CreatingWindow(hwnd, &lParam, &windowInfo);
            CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
            FILE *pFile = fopen((char *)cs->lpCreateParams, "rb");
            if(pFile)
            {
                InitText(&text, pFile);
                fclose(pFile);
            }
        }
        case WM_COMMAND:{
            updateMenu(&text, &windowInfo, LOWORD(wParam), hwnd, wParam);
            break;
        }

        case WM_ERASEBKGND:{
            break;
        }
        case WM_PAINT:
        {
            HDC hdc = BeginPaint(hwnd, &windowInfo.ps);
            GetClientRect(hwnd, &windowInfo.rect);

            Clean_Window(windowInfo.hMemDC);
            PrintText(&text, windowInfo.hMemDC, windowInfo.rect.bottom);

            BitBlt(hdc, 0, 0, maxWidth, maxHeight, windowInfo.hMemDC, 0, 0, SRCCOPY);
            EndPaint(hwnd, &windowInfo.ps);
            return 0;
        }

        case WM_SIZE :
            windowInfo.VscrollMax = max(0, windowInfo.rect.bottom);
            windowInfo.VscrollPos = min(windowInfo.VscrollPos, windowInfo.VscrollMax);
            SetScrollRange(hwnd, SB_VERT, 0, windowInfo.VscrollMax, FALSE);

            windowInfo.HscrollMax = max(0, windowInfo.rect.right);
            windowInfo.HscrollPos = min(windowInfo.HscrollPos, windowInfo.HscrollMax);
            SetScrollRange(hwnd, SB_HORZ, 0, windowInfo.HscrollMax, FALSE);

            ResizeText(&text, windowInfo.rect.right);
            SetCaretPos((text.caretOffsetInLine + text.textOffset) * text.letterWidth, (text.caretLine - text.workLine) * text.letterHeight);
            break;

       case WM_VSCROLL :
            SetVerticalScroll(&text, &windowInfo, LOWORD(wParam), &wParam, hwnd);
            SetCaretPos((text.caretOffsetInLine + text.textOffset) * text.letterWidth, (text.caretLine - text.workLine) * text.letterHeight);
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_HSCROLL :
            SetHorizontalScroll(&text, &windowInfo, LOWORD(wParam), &wParam, hwnd);
            SetCaretPos((text.caretOffsetInLine + text.textOffset) * text.letterWidth, (text.caretLine - text.workLine) * text.letterHeight);
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_SETFOCUS:
            CreateCaret(hwnd, (HBITMAP)0, 2, 16);
            SetCaretPos((text.caretOffsetInLine + text.textOffset) * text.letterWidth, (text.caretLine - text.workLine) * text.letterHeight);
            ShowCaret(hwnd);
            break;

        case WM_KILLFOCUS:
            HideCaret(hwnd);
            DestroyCaret();
            break;

        case WM_KEYDOWN:
            switch (wParam)
            {
            case VK_UP:
                MoveCaretUp(&windowInfo, &text, hwnd);
                break;
            case VK_DOWN:
                MoveCaretDown(&windowInfo, &text, hwnd);
                break;
            case VK_LEFT:
                MoveCaretLeft(&windowInfo, &text, hwnd);
                break;
            case VK_RIGHT:
                MoveCaretRight(&windowInfo, &text, hwnd);
                break;
            }
            SetHorizontalScroll(&text, &windowInfo, -1, &wParam, hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            break;


        case WM_DESTROY:
            if(text.pBuffer){
                FreeFileText(&text);
                FreeText(&text);
            }
            DeleteDC(windowInfo.hMemDC);
            PostQuitMessage (0);
            break;

        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
