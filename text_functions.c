#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#include "text_functions.h"
#include "text_structs.h"


/*    IN:
        pFile - file
      OUT:
        fileSize - size of file

      get to size of size */
long getFSize(FILE *pFile) {

    long fileSize;
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    return fileSize;
}


/*loading data file in memory */
int loadText(TEXT_DATA *Text, FILE* pFile) {

    if (pFile != NULL){
        Text->bufferSize = getFSize(pFile);
        Text->pBuffer = (char *)calloc(Text->bufferSize, sizeof(char));
        if(Text->pBuffer == NULL)
        {
            FreeFileText(Text);
            return 0;
        }
        fread(Text->pBuffer, sizeof(char), Text->bufferSize, pFile);
        return 1;
    }
    return 0;
}

/*    moves caret to the beginning of the window */
void SetCaretOnFirstVisibleLine(TEXT_DATA *Text)
{
    Text->caretOffsetInLine = 0;
    Text->textOffset = 0;
    Text->caretLine = Text->workLine;
    SetCaretPos((Text->caretOffsetInLine + Text->textOffset) * Text->letterWidth, (Text->caretLine - Text->workLine) * Text->letterHeight);
}


/*    IN:
        pFile - file
      OUT:
        Text - TEXT_DATA structure

      initiates TEXT_DATA */
void InitText(TEXT_DATA *Text, FILE *pFile)
{
    int i, j, offset, lineLen;

    Text->letterHeight = 16;
    Text->letterWidth = 10;
    Text->workLine = 0;
    Text->maxLineLen = 0;
    Text->textOffset = 0;
    Text->caretLine = 0;
    Text->caretOffsetInLine = 0;
    // load file
    if (loadText(Text, pFile) == 0) {
        free(Text->pBuffer);
        Text->pBuffer = NULL;
        return;
    }

    // calculate linesSize
    Text->defaultLinesSize = 0;
    for (i = 0; i < Text->bufferSize; ++i)
    {
        if (Text->pBuffer[i] == '\n' || Text->pBuffer[i] == '\0')
            ++Text->defaultLinesSize;
        if (Text->pBuffer[i] == '\t')
            Text->pBuffer[i] = ' ';
        if (Text->pBuffer[i] == '\0')
            break;
    }

    Text->bufferSize = i;
    Text->workLinesBufferSize = Text->workLinesSize = Text->defaultLinesSize;
    Text->pDefaultLines = (long *)malloc((Text->defaultLinesSize + 1) * sizeof(long));
    Text->pWorkLines = (long *)malloc((Text->workLinesSize + 1) * sizeof(long));

    // initialization defoult lines
    lineLen = 0;
    offset = 0;
    Text->pWorkLines[0] = Text->pDefaultLines[0] = 0;
    for (i = 0, j = 1; i < Text->bufferSize; ++i)
    {
        if (offset > 0)
         Text->pBuffer[i] = Text->pBuffer[i + offset];

        lineLen++;

        if (Text->pBuffer[i] == '\n' || Text->pBuffer[i] == '\0')
        {
            if (lineLen > Text->maxLineLen)
            Text->maxLineLen = lineLen;
            lineLen = 0;
            Text->pWorkLines[j] = Text->pDefaultLines[j] = i;
            j++;
        }

        if (Text->pBuffer[i] == '\r' || Text->pBuffer[i] == '\n') {
            offset++;
            Text->pBuffer[i] = Text->pBuffer[i + offset];
            i--;
            Text->bufferSize--;
        }
    }

    Text->pWorkLines[Text->workLinesSize] = Text->bufferSize;
    Text->pDefaultLines[Text->defaultLinesSize] = Text->bufferSize;

    Text->alignment = 1;
    ResizeText(Text, Text->maxLineLen * Text->letterWidth);
    Text->alignment = 0;
    SetCaretOnFirstVisibleLine(Text);
}


/*    IN:
        Text - TEXT_DATA  structure
        hMemDC - buffer for TextOut
        Height - window Height
      OUT: -

      text output on the screen */
void PrintText(TEXT_DATA *Text, const HDC hMemDC, int Height)
{
    int i;

    for (i = 0; Text->letterHeight * i < Height && Text->workLine + i < Text->workLinesSize; ++i)
    {
        TextOutA(hMemDC,
                 Text->textOffset * Text->letterWidth,
                 Text->letterHeight * i,
                 Text->pBuffer + Text->pWorkLines[Text->workLine + i],
                 (Text->pWorkLines[Text->workLine + i + 1] - Text->pWorkLines[Text->workLine + i]));
    }
}

/*    IN:
        Text - TEXT_DATA structure
      OUT: -

      memory free */
void FreeText(TEXT_DATA *Text)
{
    free(Text->pDefaultLines);
    free(Text->pWorkLines);
    free(Text->pBuffer);
    free(Text);
}

/*    IN:
        Text - TEXT_DgraphvizATA structure
      OUT: -

      memory free */
void FreeFileText(TEXT_DATA *Text)
{
    free(Text->pDefaultLines);
    free(Text->pWorkLines);
    free(Text->pBuffer);
}

/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        newWindowWidth - window Width in symbols
      OUT: -

      resize text by incoming width */
void ResizeText(TEXT_DATA *Text, int newWindowWidth)
{
    int count, i, j, caretBufferPos, gotCaret = 1, isCaretOnEnd = 0;
    if (Text->pBuffer == NULL || Text->alignment == 0)
        return;

    int currentBufferPos = Text->pWorkLines[Text->workLine];
    Text->workLinesSize = 0;

    if (Text->caretOffsetInLine == (Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine]) && Text->caretOffsetInLine != 0)
        isCaretOnEnd = 1;

    count = newWindowWidth / Text->letterWidth;

    caretBufferPos = Text->pWorkLines[Text->caretLine] + Text->caretOffsetInLine;

    for (i = 0; i < Text->defaultLinesSize; ++i) // count work lines
    {
        Text->workLinesSize += ceil((double)(Text->pDefaultLines[i + 1] - Text->pDefaultLines[i]) / count);
        if (Text->pDefaultLines[i + 1] - Text->pDefaultLines[i] == 0)
            Text->workLinesSize++;
    }
    if (Text->workLinesBufferSize < Text->workLinesSize)
    {
        Text->workLinesBufferSize = Text->workLinesSize;
        Text->pWorkLines = (long *)realloc(Text->pWorkLines, (Text->workLinesSize + 2) * sizeof(long));
    }
    for (i = 0, j = 0; i < Text->defaultLinesSize; ++i) //j - work i - default
    {
        int currWorkLineLenUsed = 0, k;

        if (Text->pDefaultLines[i + 1] - Text->pDefaultLines[i] == 0)
        {
            Text->pWorkLines[j] = Text->pDefaultLines[i];
            j++;
        }

        for (k = 0; (Text->pDefaultLines[i + 1] - Text->pDefaultLines[i]) != currWorkLineLenUsed && j < Text->workLinesSize; ++k, ++j) // make work lines from default lines
        {
            int lineLen;
            Text->pWorkLines[j] = Text->pDefaultLines[i] + currWorkLineLenUsed;
            if ((Text->pDefaultLines[i + 1] - Text->pDefaultLines[i]) - currWorkLineLenUsed < count)
                lineLen = Text->pDefaultLines[i + 1] - Text->pDefaultLines[i] - currWorkLineLenUsed;
            else
                lineLen = count;

            if (Text->pWorkLines[j] < currentBufferPos && Text->pWorkLines[j] + lineLen > currentBufferPos)
            {
                lineLen = currentBufferPos - Text->pWorkLines[j];
            }
            if (Text->pWorkLines[j] == currentBufferPos)
                Text->workLine = j;
            currWorkLineLenUsed += lineLen;

            if (j > 0 && Text->pWorkLines[j - 1] < caretBufferPos && Text->pWorkLines[j] > caretBufferPos && gotCaret)
            {
                Text->caretLine = j - 1;
                Text->caretOffsetInLine = caretBufferPos - Text->pWorkLines[j - 1];
                gotCaret = 0;
            }

            if (j > 0 && Text->pWorkLines[j] == caretBufferPos && gotCaret)
            {
                if (isCaretOnEnd)
                {
                    Text->caretLine = j - 1;
                    Text->caretOffsetInLine = caretBufferPos - Text->pWorkLines[j - 1];
                }
                else
                {
                    Text->caretLine = j;
                    Text->caretOffsetInLine = 0;
                }
            }
        }
        if ((int)ceil((double)(Text->pDefaultLines[i + 1] - Text->pDefaultLines[i]) / count) != k)
            Text->workLinesSize++;
    }
    if (Text->pWorkLines[j - 1] <= caretBufferPos && Text->bufferSize >= caretBufferPos && gotCaret)
    {
        Text->caretLine = j - 1;
        Text->caretOffsetInLine = caretBufferPos - Text->pWorkLines[j - 1];
        gotCaret = 0;
    }
    Text->pWorkLines[Text->workLinesSize] = Text->bufferSize;
}


/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - WINDOW_INFO structure
        hwnd - window descriptor
      returns: -

      shift screen to the caret if it was moved with scrolls */
void MoveCaretIntoView(TEXT_DATA *Text, WINDOW_INFO *windowInfo, HWND hwnd)
{
    //vertical
    if (Text->caretLine < Text->workLine)
        Text->workLine = Text->caretLine;
    if (Text->caretLine > Text->workLine + windowInfo->rect.bottom / Text->letterHeight - 1)
        Text->workLine = Text->caretLine;

    //horizontal
    if ((-Text->textOffset) - Text->caretOffsetInLine > 0)
        Text->textOffset = -Text->caretOffsetInLine;
    if (Text->caretOffsetInLine + Text->textOffset > windowInfo->rect.right / Text->letterWidth)
        Text->textOffset = max(-Text->caretOffsetInLine, -(Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine] - windowInfo->rect.right / Text->letterWidth));

    SetScrollPos(hwnd, SB_HORZ, Text->textOffset, TRUE);
    SetScrollPos(hwnd, SB_VERT, Text->workLine, TRUE);
}

/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - WINDOW_INFO structure
        hwnd - window descriptor
      OUT: -

    moves caret UP or shifts it if there is no symbols in destination line */
void MoveCaretUp(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd)
{
    Text->caretLine--;

    if (Text->caretLine >= 0)
    {
        if (Text->caretLine < Text->workLine)
            SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
        if (Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine] < Text->caretOffsetInLine)
            Text->caretOffsetInLine = Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine];

        if (Text->caretOffsetInLine > windowInfo->rect.right / Text->letterWidth)
        {
            Text->textOffset = (Text->caretOffsetInLine) + windowInfo->rect.right / Text->letterWidth;
        }
        else
        {
            Text->textOffset = 0;
        }
    }
    else
    {
        Text->caretLine++;
    }
    MoveCaretIntoView(Text, windowInfo, hwnd);
    SetCaretPos((Text->caretOffsetInLine + Text->textOffset) * Text->letterWidth, (Text->caretLine - Text->workLine) * Text->letterHeight);
}


/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - WINDOW_INFO structure
        hwnd - window descriptor
      OUT: -

      moves caret DOWN or shifts it if there is no symbols in destination line */
void MoveCaretDown(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd)
{
    Text->caretLine++;

    if (Text->caretLine < Text->workLinesSize)
    {
        if (Text->caretLine >= Text->workLine + windowInfo->rect.bottom / Text->letterHeight)
            SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
        if (Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine] < Text->caretOffsetInLine)
            Text->caretOffsetInLine = Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine];

        if (Text->caretOffsetInLine > windowInfo->rect.right / Text->letterWidth)
        {
            Text->textOffset = (-Text->caretOffsetInLine) + windowInfo->rect.right / Text->letterWidth;
        }
        else
        {
            Text->textOffset = 0;
        }
    }
    else
    {
        Text->caretLine--;
    }

    MoveCaretIntoView(Text, windowInfo, hwnd);
    SetCaretPos((Text->caretOffsetInLine + Text->textOffset) * Text->letterWidth, (Text->caretLine - Text->workLine) * Text->letterHeight);
}


/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - WINDOW_INFO structure
        hwnd - window descriptor
      OUT: -

      moves caret LEFT */
void MoveCaretLeft(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd)
{
    Text->caretOffsetInLine--;

    if (Text->caretOffsetInLine < 0)
    {
        if (Text->caretLine != 0)
        {
            Text->caretLine--;
            if (Text->caretLine < Text->workLine)
            {
                SendMessage(hwnd, WM_HSCROLL, SB_LINEUP, 0L);
            }
            Text->caretOffsetInLine = Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine];

            if (Text->caretOffsetInLine > windowInfo->rect.right / Text->letterWidth)
            {
                Text->textOffset = (-Text->caretOffsetInLine) + windowInfo->rect.right / Text->letterWidth;
            }
            else
            {
                Text->textOffset = 0;
            }
            SetScrollPos(hwnd, SB_HORZ, Text->textOffset, TRUE);
        }
        else
        {
           Text->caretOffsetInLine++;
        }
    }
    else if (Text->caretOffsetInLine + Text->textOffset < 0)
    {
        SendMessage(hwnd, WM_HSCROLL, SB_LINELEFT, 0L);
    }
    MoveCaretIntoView(Text, windowInfo, hwnd);
    SetCaretPos((Text->caretOffsetInLine + Text->textOffset) * Text->letterWidth, (Text->caretLine - Text->workLine) * Text->letterHeight);
}



/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - WINDOW_INFO structure
        hwnd - window descriptor
      OUT: -

      moves caret RIGHT */
void MoveCaretRight(WINDOW_INFO *windowInfo, TEXT_DATA *Text, HWND hwnd)
{
    Text->caretOffsetInLine++;

    if (Text->caretOffsetInLine > (Text->pWorkLines[Text->caretLine + 1] - Text->pWorkLines[Text->caretLine]))
    {
        if (Text->caretLine != Text->workLinesSize - 1)
        {
            Text->caretLine++;
            if (Text->caretLine > (Text->workLine + windowInfo->rect.bottom / Text->letterHeight))
            {
                SendMessage(hwnd, WM_HSCROLL, SB_LINEDOWN, 0L);
            }

            Text->caretOffsetInLine = 0;
            Text->textOffset = 0;
            SetScrollPos(hwnd, SB_HORZ, Text->textOffset, TRUE);
        }
        else
        {
           Text->caretOffsetInLine--;
        }
    }
    else if (Text->caretOffsetInLine + Text->textOffset > windowInfo->rect.right / Text->letterWidth)
    {
        SendMessage(hwnd, WM_HSCROLL, SB_LINERIGHT, 0L);
    }

    MoveCaretIntoView(Text, windowInfo, hwnd);
    SetCaretPos((Text->caretOffsetInLine + Text->textOffset) * Text->letterWidth, (Text->caretLine - Text->workLine) * Text->letterHeight);
}


/*    IN/OUT:
        Text - structure TEXT_DATA
      IN:
        windowInfo - structure WINDOW_INFO
        SB_ID - type of scroll
        wParam - word parameters
        hwnd - window descriptor
      OUT: -

      vertical text scroll */
void SetVerticalScroll(TEXT_DATA *Text, WINDOW_INFO *windowInfo, int SB_ID, WPARAM *wParam, HWND hwnd)
{
    ResizeText(Text, windowInfo->rect.right);

    switch (SB_ID)
    {
    case SB_TOP:
        Text->workLine = 0;
        SetCaretOnFirstVisibleLine(Text);
        break;
    case SB_BOTTOM:
        Text->workLine = Text->workLinesSize - 1;
        if (Text->workLinesSize <= windowInfo->rect.bottom / Text->letterHeight)
        {
            Text->workLine = 0;
        }
        else
        {
            Text->workLine = Text->workLinesSize - windowInfo->rect.bottom / Text->letterHeight;
        }
        Text->caretOffsetInLine = 0;
        Text->textOffset = 0;
        Text->caretLine = Text->workLinesSize - 1;
        SetCaretPos((Text->caretOffsetInLine + Text->textOffset) * Text->letterWidth, (Text->caretLine - Text->workLine) * Text->letterHeight);
        break;
    case SB_LINEUP:
        Text->workLine--;
        break;
    case SB_LINEDOWN:
        Text->workLine++;
        break;
    case SB_PAGEUP:
        Text->workLine -= windowInfo->rect.bottom / Text->letterHeight;
        if (Text->workLine < 0)
            Text->workLine = 0;
        SetCaretOnFirstVisibleLine(Text);
        break;
    case SB_PAGEDOWN:
        Text->workLine += windowInfo->rect.bottom / Text->letterHeight;
        if (Text->workLine > Text->workLinesSize - 1)
            Text->workLine = Text->workLinesSize - 1;
        SetCaretOnFirstVisibleLine(Text);
        break;
    case SB_THUMBTRACK:
    {
        int scrollPos = HIWORD(*wParam);
        Text->workLine = (double)scrollPos / windowInfo->VscrollMax * Text->workLinesSize;
        break;
    }
    }
    SetScrollPos(hwnd, SB_VERT, (int)((double)Text->workLine / Text->workLinesSize * windowInfo->VscrollMax), TRUE);
}


/*    IN/OUT:
        Text - TEXT_DATA structure
      IN:
        windowInfo - WINDOW_INFO structure
        SB_ID - type of scroll
        wParam - word parameters
        hwnd - window descriptor
      OUT: -

      horizontal text scroll */
void SetHorizontalScroll(TEXT_DATA *Text, WINDOW_INFO *windowInfo, int SB_ID, WPARAM *wParam, HWND hwnd)
{
    if (Text->alignment == 1)
    {
        Text->textOffset = 0;
        return;
    }
    switch (SB_ID)
    {
    case SB_LINELEFT:
        Text->textOffset = min(Text->textOffset + 1, 0);
        break;
    case SB_LINERIGHT:
        Text->textOffset = max(Text->textOffset - 1, -(Text->maxLineLen));
        break;
    case SB_THUMBTRACK:
    {
        int scrollPos = HIWORD(*wParam);
        Text->textOffset = -(double)scrollPos / windowInfo->HscrollMax * Text->maxLineLen;
        break;
    }
    }
    SetScrollPos(hwnd, SB_HORZ, (int)((double)abs(Text->textOffset) / Text->maxLineLen * windowInfo->HscrollMax), TRUE);
}

