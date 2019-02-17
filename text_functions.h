#pragma once
#include "text_structs.h"
#include <stdio.h>


void InitText(TEXT_DATA *Text, FILE *pFile);
void FreeText(TEXT_DATA *Text);
void FreeFileText(TEXT_DATA *Text);

void PrintText(TEXT_DATA *Text, HDC hMemDC, int Height);
void ResizeText(TEXT_DATA *Text, int newWindowWidth);

void SetVerticalScroll(TEXT_DATA *Text, WINDOW_INFO *windowInfo, int SB_ID, WPARAM *wParam, HWND hwnd);
void SetHorizontalScroll(TEXT_DATA *Text, WINDOW_INFO *windowInfo, int SB_ID, WPARAM *wParam, HWND hwnd);

void SetCaretOnFirstVisibleLine(TEXT_DATA *Text);
void MoveCaretUp(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd);
void MoveCaretDown(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd);
void MoveCaretLeft(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd);
void MoveCaretRight(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd);
void UpdateCaretPos(TEXT_DATA *Text);
void MoveCaretIntoView(TEXT_DATA *Text, WINDOW_INFO *windowInfo, HWND hwnd);
