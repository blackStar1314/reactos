#pragma once

#define FLASHW_MASK         0x0000000f
#define FLASHW_SYSTIMER     0x00000400
#define FLASHW_FINISHED     0x00000800
#define FLASHW_STARTED      0x00001000
#define FLASHW_COUNT        0x00002000
#define FLASHW_KILLSYSTIMER 0x00004000
#define FLASHW_ACTIVE       0x00008000

typedef struct _FLASHDATA{
  DWORD FlashState;
  UINT uCount;
} FLASHDATA, *PFLASHDATA;

BOOL FASTCALL co_UserRedrawWindow(PWND Wnd, const RECTL* UpdateRect, PREGION UpdateRgn, ULONG Flags);
VOID FASTCALL IntInvalidateWindows(PWND Window, PREGION Rgn, ULONG Flags);
BOOL FASTCALL IntGetPaintMessage(PWND Window, UINT MsgFilterMin, UINT MsgFilterMax, PTHREADINFO Thread, MSG *Message, BOOL Remove);
INT FASTCALL UserRealizePalette(HDC);
INT FASTCALL co_UserGetUpdateRgn(PWND, PREGION, BOOL);
VOID FASTCALL co_IntPaintWindows(PWND Window, ULONG Flags, BOOL Recurse);
BOOL FASTCALL IntValidateParent(PWND Child, PREGION ValidateRgn, BOOL Recurse);
BOOL FASTCALL IntIsWindowDirty(PWND);
BOOL FASTCALL IntEndPaint(PWND,PPAINTSTRUCT);
HDC FASTCALL IntBeginPaint(PWND,PPAINTSTRUCT);
PCURICON_OBJECT FASTCALL NC_IconForWindow( PWND );
BOOL FASTCALL IntFlashWindowEx(PWND,PFLASHWINFO);
