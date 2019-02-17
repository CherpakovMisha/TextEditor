#pragma once
#include <Windows.h>

#define maxHeight 1080
#define maxWidth 1800

typedef struct WINDOW_INFO {

    HDC         hMemDC;         // device context descriptor
    PAINTSTRUCT ps;             // drawing window information
    RECT        rect;           // window rectangle
    double      VscrollPos, VscrollMax, HscrollPos, HscrollMax; // Scroll data

}WINDOW_INFO;

typedef struct TEXT_DATA
{
    char *pBuffer;                     // buffer of income text
    int  bufferSize;                   // size of buffer
    int  letterWidth, letterHeight;    // width and height of symbols
    int  alignment;                    // need word wrap or not
    int  textOffset;                   // horizontal offset

    long *pDefaultLines;               // array of lines offsets in original text
    long *pWorkLines;                  // array of lines offsets after alignment
    long defaultLinesSize;             // pDefaultLines array size
    long workLinesSize;                // pWorkLines array size
    long workLinesBufferSize;          // additional buffer size for pWorkLines
    long workLine;                     // number of first line on screen in pWorkLines
    long maxLineLen;                   // maximum line length

    int caretLine;                     // number of the current line for caret
    int caretOffsetInLine;             // offset of caret from beginning of the line
}TEXT_DATA;

