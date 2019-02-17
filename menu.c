#include <stdio.h>
#include "text_functions.h"
#include "text_structs.h"
#include "menu.h"


void InitOFN(OPENFILENAME * ofn, HWND * hwnd)
{
    ZeroMemory(ofn, sizeof(*ofn));
    static CHAR szFilter[] = "Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0";
    ofn->lStructSize = sizeof(*ofn);
    ofn->hwndOwner = *hwnd;
    ofn->lpstrFile = "\0";
    ofn->nMaxFile = 100;
    ofn->lpstrFilter = (LPCSTR)szFilter;
    ofn->nFilterIndex = 1;
    ofn->lpstrTitle = TEXT("Open");
    ofn->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
 }


void updateMenu(TEXT_DATA *Text, WINDOW_INFO *windowInfo, int MENU_ID, HWND hwnd, WPARAM wParam)
{
    HMENU hMenu = GetMenu(hwnd);
    switch (MENU_ID)
    {
        case IDM_OPEN: //open file
        {
            FreeFileText(Text);

            OPENFILENAME ofn;
            char nameFile[100] = { 0 };
            InitOFN(&ofn, &hwnd);
            ofn.lpstrFile = nameFile;
            windowInfo->VscrollPos = 0;
            windowInfo->HscrollMax = 0;
            if (GetOpenFileName(&ofn))
            {
                FILE *pFile;
                pFile = fopen(ofn.lpstrFile, "rb");
                InitText(Text, pFile);
                fclose(pFile);
            }
            SendMessage(hwnd, WM_SIZE, 0, 0L);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

        case IDM_DEFAULT: //standart view
        {
            ResizeText(Text, Text->maxLineLen * Text->letterWidth);
            Text->alignment = 0;
            SendMessage(hwnd, WM_SIZE, 0, 0L);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

        case IDM_JUSTIFY: // format view
        {
            Text->alignment = 1;
            Text->textOffset = 0;
            windowInfo->HscrollMax = 0;
            SetScrollPos(hwnd, SB_HORZ, 0, TRUE);
            SendMessage(hwnd, WM_SIZE, 0, 0L);
            InvalidateRect(hwnd, NULL, TRUE);

        }
        break;


        case IDM_EXIT:
        {
            PostQuitMessage(0);
        }
    }
}
