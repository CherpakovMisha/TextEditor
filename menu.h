#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "text_structs.h"

#define MYMENU 101

#define IDM_OPEN    1
#define IDM_EXIT    2
#define IDM_DEFAULT 3
#define IDM_JUSTIFY 4



void updateMenu(TEXT_DATA *Text, WINDOW_INFO *windowInfo, int MENU_ID, HWND hwnd, WPARAM wParam);
#endif // MENU_H_INCLUDED
