#include <windows.h>
#include "conmsgbox.h"

#pragma warning(disable : 4244)

void clip(LPCWSTR txt)
{
	SIZE_T l = wcslen(txt) + 1;
	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, l * sizeof(wchar_t));
	memcpy(GlobalLock(hmem), txt, l * sizeof(wchar_t));
	GlobalUnlock(hmem);
	OpenClipboard(NULL);
	EmptyClipboard(); // Using this with OpenClipboard(NULL) should not work, but it does! Magick?
	SetClipboardData(CF_UNICODETEXT, hmem);
	CloseClipboard();
}

HANDLE ih;
HANDLE oh;

void pcp(wchar_t c, int n, int x, int y)
{
	COORD p;
	p.X = x;
	p.Y = y;
	SetConsoleCursorPosition(oh, p);
	while(--n >= 0)
	{
		WriteConsoleW(oh, &c, 1, NULL, NULL);
	}
}

void pcc(wchar_t c, int n)
{
	while(--n >= 0)
	{
		WriteConsoleW(oh, &c, 1, NULL, NULL);
	}
}

//_______________
//|             |
//|  TALL BUTT  |
//|_____________|

//| SHORT BUTT |

#define MAX_SCR_W 119
#define MAX_SCR_H 31
#define MIN_SCR_H 15

#define CONBUTT_NUM     3
#define DIST_BTW_BUTTS  2
#define DIST_MAIN_EXTRA 1
#define DIST_EXTRA_TEXT 1
#define BT_TALL_HEIGHT  4
#define BT_SHORT_HEIGHT 1

#define BT_NORM 0x0
#define BT_HOVR 0x1
#define BT_PRST 0x2

#define BT_TALL  0x0
#define BT_SHORT 0x1

SHORT min_scr_w;
SHORT header_h;

typedef struct
{
	SHORT x;
	SHORT y;
	SHORT xt; // x top of collision rectangle
	SHORT yt; // y top
	SHORT xb; // x bottom
	SHORT yb; // y bottom
	LPCWSTR txt;
	int ret; // Return code
	SIZE_T w; // Width
	SHORT st; // Button state
	SHORT sz; // Button size
} CONBUTT;

void pButt(CONBUTT *b)
{
	CONSOLE_SCREEN_BUFFER_INFO scr_inf;
	GetConsoleScreenBufferInfo(oh, &scr_inf);
	WORD def = scr_inf.wAttributes, hlight = def;

	if(b->st != BT_NORM)
	{
		hlight = b->st & BT_PRST ? FOREGROUND_INTENSITY | BACKGROUND_INTENSITY |
			BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE :
			BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	}

	int txt_pad = 2, txt_y_offset = 2;
	SIZE_T l = wcslen(b->txt);
	if(b->sz == BT_TALL)
	{
		pcp('_', l + 6, b->x, b->y);
		pcp('|', 1, b->x, b->y+1);
		SetConsoleTextAttribute(oh, hlight);
		pcc(' ', l + 4);
		SetConsoleTextAttribute(oh, def);
		pcc('|', 1);
	}
	else
	{
		txt_pad = 1;
		txt_y_offset = 0;
	}

	pcp('|', 1, b->x, b->y + txt_y_offset);
	SetConsoleTextAttribute(oh, hlight);
	pcc(' ', txt_pad);
	LPCWSTR t = b->txt - 1;
	while(*(++t) != 0)
	{
		pcc(*t, 1);
	}
	pcc(' ', txt_pad);
	SetConsoleTextAttribute(oh, def);
	pcc('|', 1);

	if(b->sz == BT_TALL)
	{
		pcp('|', 1, b->x, b->y+3);
		SetConsoleTextAttribute(oh, hlight);
		pcc('_', l + 4);
		SetConsoleTextAttribute(oh, def);
		pcc('|', 1);
	}
}

SIZE_T wButt(LPCWSTR txt, int size) // Button width
{
	SIZE_T l = wcslen(txt);
	int txt_pad = size == BT_TALL ? 2 : 1;

	return l + 2 + txt_pad * 2;
}

void calcLines(SIZE_T *max_line, SIZE_T *line, SIZE_T *lines)
{
	*max_line = *line > *max_line ? *line : *max_line;
	if(*line > MAX_SCR_W)
	{
		*lines += *line/MAX_SCR_W;
		*lines += *line % MAX_SCR_W > 0 ? 1 : 0;
	}
	else
	{
		++(*lines);
	}
}

void scrBufSize(LPCWSTR txt, SMALL_RECT *scr, COORD *buf, SIZE_T *len)
{
	SIZE_T lines = 0;
	SIZE_T line = 0;
	SIZE_T max_line = 0;
	--txt;
	while(*(++txt) != 0)
	{
		++line;
		++(*len);
		if(*txt == '\n')
		{
			calcLines(&max_line, &line, &lines);
			line = 0;
		}
	}

	if(*(--txt) != '\n') // No terminating newline found
	{
		calcLines(&max_line, &line, &lines);
	}

	lines += header_h;

	scr->Left = 0;
	scr->Top = 0;
	scr->Right = max_line < min_scr_w ? min_scr_w : max_line > MAX_SCR_W ? MAX_SCR_W : max_line;
	scr->Bottom = lines < MIN_SCR_H ? MIN_SCR_H : lines > MAX_SCR_H ? MAX_SCR_H : lines;

	buf->X = scr->Right + 1;
	buf->Y = lines < MIN_SCR_H ? MIN_SCR_H + 1 : lines > 0x7FFF ? 0x7FFF : lines + 1;
}

int consoleMessageBox(LPCWSTR text, LPCWSTR title)
{
	static int ret_val = -1;

	// Create main buttons
	static CONBUTT b[CONBUTT_NUM];
	b[0].x = 2;
	b[0].y = 0;
	b[0].txt = L"ABORT";
	b[0].ret = IDABORT;

	b[1].txt = L"RETRY";
	b[1].ret = IDRETRY;

	b[2].txt = L"IGNORE";
	b[2].ret = IDIGNORE;

	min_scr_w = 0;
	for(int i = 0; i < CONBUTT_NUM; ++i)
	{
		if(i > 0)
		{
			b[i].x = b[i-1].x + b[i-1].w + DIST_BTW_BUTTS;
			b[i].y = b[i-1].y;
		}

		b[i].st = BT_NORM;
		b[i].sz = BT_TALL;
		b[i].w = wButt(b[i].txt, b[i].sz);
		b[i].xt = b[i].x;
		b[i].yt = b[i].y;
		b[i].xb = b[i].x + b[i].w;
		b[i].yb = b[i].y + BT_TALL_HEIGHT;

		min_scr_w += b[i].w + DIST_BTW_BUTTS;
	}
	min_scr_w += b[0].x;
	--min_scr_w;

	header_h = b[0].y + BT_TALL_HEIGHT + DIST_MAIN_EXTRA + BT_SHORT_HEIGHT + DIST_EXTRA_TEXT;

	// Create extra buttons
	static CONBUTT clip_b;
	clip_b.txt = L"COPY TO CLIPBOARD";
	clip_b.ret = -1;
	clip_b.st = BT_NORM;
	clip_b.sz = BT_SHORT;
	clip_b.w = wButt(clip_b.txt, clip_b.sz);
	clip_b.x = clip_b.w < min_scr_w ? (min_scr_w - clip_b.w)/2 : 0;
	clip_b.y = b[0].y + BT_TALL_HEIGHT + DIST_MAIN_EXTRA;
	clip_b.xt = clip_b.x - 1;
	clip_b.yt = clip_b.y - 1;
	clip_b.xb = clip_b.x + clip_b.w;
	clip_b.yb = clip_b.y + BT_SHORT_HEIGHT;

	// Adjust console size and buffer
	COORD def_sz, sz;
	SMALL_RECT def_scrsz, scrsz;
	SIZE_T txtl = 0;
	scrBufSize(text, &scrsz, &sz, &txtl);

	FreeConsole();
	AllocConsole();

	SetConsoleTitleW(title);

	ih = GetStdHandle(STD_INPUT_HANDLE);
	oh = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleWindowInfo(oh, TRUE, &scrsz);
	SetConsoleScreenBufferSize(oh, sz);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(oh, &csbi);
	def_sz = csbi.dwSize;
	def_scrsz = csbi.srWindow;

	// Hide cursor
	CONSOLE_CURSOR_INFO def_cur;
	GetConsoleCursorInfo(oh, &def_cur);
	CONSOLE_CURSOR_INFO new_cur;
	new_cur.dwSize = 100;
	new_cur.bVisible = FALSE;
	SetConsoleCursorInfo(oh, &new_cur);

	DWORD def_con_mode;
	GetConsoleMode(ih, &def_con_mode);
	SetConsoleMode(ih, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);

	// Display all buttons
	for(int i = 0; i < CONBUTT_NUM; ++i)
	{
		pButt(&b[i]);
	}

	pButt(&clip_b);

	// Finally, display the text
	COORD p, z;
	p.X = 0;
	p.Y = b[0].y + header_h;
	z.X = 0;
	z.Y = 0;

	SetConsoleCursorPosition(oh, p);
	WriteConsoleW(oh, text, txtl, NULL, NULL);
	SetConsoleCursorPosition(oh, z);

	SetConsoleWindowInfo(oh, TRUE, &scrsz);
	SetConsoleScreenBufferSize(oh, sz);

	INPUT_RECORD ir[128];
	DWORD nread;
	int lmb_st = BT_HOVR;
	CONBUTT *hov_cb = NULL; // Currently hovered console button
	while(1)
	{
		ReadConsoleInput(ih, ir, 128, &nread);
		for(DWORD i = 0; i < nread; ++i)
		{
			if(ir[i].EventType == MOUSE_EVENT)
			{
				const MOUSE_EVENT_RECORD *m = &ir[i].Event.MouseEvent;

				COORD c = m->dwMousePosition;
				int x = c.X;
				int y = c.Y;

				// Handle main buttons
				CONBUTT *pb = b - 1;
				CONBUTT *pbe = b + CONBUTT_NUM;
				while(++pb <= pbe && pb != &clip_b)
				{
					if(pb == pbe) // Sneak in extra button
					{
						pb = &clip_b;
					}

					if(lmb_st == BT_PRST && pb != hov_cb) // Ignore other buttons if LMB pressed
					{
						continue;
					}

					if(x > pb->xt && y > pb->yt
						&& x < pb->xb && y < pb->yb) // Collision detection core
					{
						if(!(pb->st & BT_HOVR) || !(pb->st & BT_PRST))
						{
							pb->st |= BT_HOVR | lmb_st;
							pButt(pb);
							ret_val = pb->ret;
							hov_cb = pb;
						}
					}
					else
					{
						if(pb->st & BT_HOVR)
						{
							pb->st &= ~(BT_PRST | lmb_st);
							pButt(pb);
							ret_val = -1;
						}
					}
				}

				if(m->dwEventFlags == MOUSE_MOVED || hov_cb == NULL)
				{
					continue;
				}

				if(m->dwButtonState & 0x1) // LMB is beeing pressed
				{
					lmb_st = BT_PRST;

					if(hov_cb->st & BT_HOVR)
					{
						if(!(hov_cb->st & BT_PRST))
						{
							hov_cb->st |= BT_PRST;
							pButt(hov_cb);
							ret_val = hov_cb->ret;
						}
					}
				}
				else if(!(m->dwButtonState & 0x1)) // LMB was released
				{
					lmb_st = BT_HOVR;

					if(hov_cb->st & BT_HOVR)
					{
						if(ret_val > 0)
						{
							goto exit;
						}
						else // Only one special button, no need to check which one
						{
							clip(text);
						}
					}

					if(hov_cb->st & BT_PRST)
					{
						hov_cb->st &= ~BT_PRST;
						pButt(hov_cb);
						ret_val = -1;
					}
				}
			}
		}
	}

exit:
	SetConsoleCursorInfo(oh, &def_cur);
	SetConsoleMode(ih, def_con_mode);
	SetConsoleWindowInfo(oh, TRUE, &def_scrsz);
	SetConsoleScreenBufferSize(oh, def_sz);
	FreeConsole();
	return ret_val;
}