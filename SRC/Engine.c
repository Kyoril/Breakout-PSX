
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libcd.h>

#include "Engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} Color;

/* Determines whether a character has a glyph for the given font. */
#define CHAR_VALID(c) (CharIndex[c] != 255)
/* Gets a character's X coordinate for the given font. */
#define CHAR_X(c) CharData[(int)CharIndex[c] * 7]
/* Gets a character's Y coordinate for the given font. */
#define CHAR_Y(c) CharData[(int)CharIndex[c] * 7 + 1]
/* Gets a character's width for the given font. */
#define CHAR_W(c) CharData[(int)CharIndex[c] * 7 + 2]
/* Gets a character's height for the given font. */
#define CHAR_H(c) CharData[(int)CharIndex[c] * 7 + 3]
/* Gets a character's x offset for the given font. */
#define CHAR_XOFF(c) CharData[(int)CharIndex[c] * 7 + 4]
/* Gets a character's y offset for the given font. */
#define CHAR_YOFF(c) CharData[(int)CharIndex[c] * 7 + 5]
/* Gets a character's x advance value for the given font. */
#define CHAR_ADV(c) CharData[(int)CharIndex[c] * 7 + 6]

/* 
 * Contains data for font glyphs in VRAM. Each row contains the following 7 columns in order:
 * 
 *   X: X coordinate of the glyph in it's tpage
 *   Y: Y coordinate of the glyph in it's tpage
 *   W: Width of the glyph in pixels
 *   H: Height of the glyph in pixels
 *   XOffset: Offset applied to the cursor position on the x axis when rendering that glyph
 *   YOffset: Offset applied to the cursor position on the y axis when rendering that glyph
 *   XAdvance: Amount of pixels that the cursor is moved when rendering that glyph
 */
short CharData[] = {
	0, 0, 0, 0, 0, 0, 3, 
	63, 75, 6, 13, -1, -2, 3, 
	34, 89, 8, 9, -2, -2, 4, 
	39, 32, 14, 14, -2, -2, 11, 
	43, 89, 6, 9, -2, -2, 2, 
	8, 0, 8, 16, -2, -2, 4, 
	17, 0, 8, 16, -2, -2, 4, 
	101, 102, 9, 9, -2, -2, 5, 
	89, 102, 11, 10, -2, 0, 7, 
	62, 89, 6, 8, -1, 5, 3, 
	69, 89, 8, 6, -2, 3, 4, 
	78, 89, 6, 6, -1, 5, 3, 
	43, 0, 10, 14, -2, -2, 7, 
	54, 32, 8, 13, -1, -2, 7, 
	63, 32, 10, 13, -2, -2, 7, 
	54, 0, 10, 14, -2, -2, 7, 
	74, 32, 10, 13, -2, -2, 7, 
	65, 0, 11, 14, -2, -2, 7, 
	77, 0, 10, 14, -2, -2, 7, 
	85, 32, 11, 13, -2, -2, 7, 
	88, 0, 11, 14, -2, -2, 7, 
	0, 17, 11, 14, -2, -2, 7, 
	82, 102, 6, 11, -1, 0, 3, 
	70, 75, 6, 13, -1, 0, 3, 
	50, 89, 11, 8, -2, 1, 7, 
	103, 61, 10, 13, -2, -2, 7, 
	59, 47, 12, 13, -2, -2, 7, 
	72, 47, 11, 13, -1, -2, 8, 
	100, 0, 13, 14, -2, -2, 9, 
	84, 47, 11, 13, -1, -2, 9, 
	96, 47, 11, 13, -1, -2, 8, 
	0, 61, 10, 13, -1, -2, 7, 
	68, 17, 13, 14, -2, -2, 9, 
	11, 61, 11, 13, -1, -2, 9, 
	23, 61, 6, 13, -1, -2, 3, 
	82, 17, 9, 14, -2, -2, 6, 
	30, 61, 11, 13, -1, -2, 8, 
	42, 61, 10, 13, -1, -2, 7, 
	53, 61, 12, 13, -1, -2, 9, 
	66, 61, 11, 13, -1, -2, 9, 
	92, 17, 13, 14, -2, -2, 9, 
	78, 61, 11, 13, -1, -2, 8, 
	0, 32, 13, 14, -2, -2, 9, 
	90, 61, 12, 13, -1, -2, 9, 
	14, 32, 12, 14, -2, -2, 8, 
	0, 75, 11, 13, -2, -2, 7, 
	27, 32, 11, 14, -1, -2, 9, 
	12, 75, 12, 13, -2, -2, 7, 
	97, 32, 16, 13, -2, -2, 11, 
	25, 75, 12, 13, -2, -2, 7, 
	38, 75, 12, 13, -2, -2, 7, 
	51, 75, 11, 13, -2, -2, 7, 
	26, 0, 8, 16, -2, -2, 3, 
	35, 0, 7, 16, -2, -2, 3, 
	77, 75, 11, 12, -2, 0, 7, 
	0, 47, 11, 13, -2, -2, 7, 
	89, 75, 10, 12, -2, 0, 6, 
	12, 17, 10, 14, -2, -2, 7, 
	100, 75, 11, 12, -2, 0, 7, 
	12, 47, 8, 13, -2, -2, 3, 
	23, 17, 10, 14, -2, 0, 7, 
	21, 47, 10, 13, -2, -2, 7, 
	106, 17, 6, 13, -2, -2, 3, 
	0, 0, 7, 16, -3, -2, 3, 
	32, 47, 10, 13, -2, -2, 6, 
	43, 47, 6, 13, -2, -2, 3, 
	0, 102, 14, 11, -2, 0, 11, 
	15, 102, 10, 11, -2, 0, 7, 
	0, 89, 11, 12, -2, 0, 7, 
	34, 17, 11, 14, -2, 0, 7, 
	46, 17, 10, 14, -2, 0, 7, 
	26, 102, 8, 11, -2, 0, 4, 
	12, 89, 10, 12, -2, 0, 7, 
	50, 47, 8, 13, -2, -2, 3, 
	23, 89, 10, 12, -2, 0, 7, 
	35, 102, 10, 11, -2, 0, 5, 
	46, 102, 13, 11, -2, 0, 9, 
	60, 102, 10, 11, -2, 0, 5, 
	57, 17, 10, 14, -2, 0, 5, 
	71, 102, 10, 11, -2, 0, 5
};

/* 
 * An array for each char (0-255) which defines the index in the font glyph data array.
 * A value of 255 (0xff) means that the char is not supported and will be ignored when
 * rendering text.
*/
u_char CharIndex[] = {
	255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 0, 1, 2, 255, 255, 3, 255, 4, 5, 
	6, 7, 8, 9, 10, 11, 255, 12, 13, 14, 
	15, 16, 17, 18, 19, 20, 21, 22, 23, 255, 
	24, 255, 25, 255, 26, 27, 28, 29, 30, 31, 
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 
	52, 255, 53, 255, 255, 255, 54, 55, 56, 57, 
	58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 
	68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 
	78, 79, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255
};

/* TIM params for font sheet. */
static GsIMAGE s_fontImage;
/* Sprite for rendering text. */
static GsSPRITE s_fontSprite;
/* Main archive. */
static PckTOC s_mainArchive;

static Color s_clearColor;

/* ordering table (OT) definition */
#define	OT_LENGTH 1
GsOT WorldOT[2];
GsOT_TAG OTTags[2][1<<OT_LENGTH];

/* GPU packet area definition */
#define	PACKETMAX 6000*24
PACKET GpuPacketArea[2][PACKETMAX];

int s_activeBuff = 0;


void SwapTo3D()
{
	GsInit3D();
	GsSetProjection(160);
}

void SwapTo2D()
{
	GsInitGraph(320, 240, GsINTER | GsOFSGPU, 1, 0);
}

void SetClearColor(u_char red, u_char green, u_char blue)
{
	s_clearColor.red = red;
	s_clearColor.green = green;
	s_clearColor.blue = blue;
}

GsIMAGE LoadTIM(u_long *tMemAddress) 
{
	RECT tRect;
	GsIMAGE tTim;

	tMemAddress++;
	GsGetTimInfo(tMemAddress, &tTim);	// Get TIM info from TIM

	tRect.x = tTim.px;
	tRect.y = tTim.py;
	tRect.w = tTim.pw;
	tRect.h = tTim.ph;
	LoadImage(&tRect, tTim.pixel);		// Load TIM data into framebuffer
	DrawSync(0);

	if ((tTim.pmode >> 3) & 0x01)
	{
		tRect.x = tTim.cx;
		tRect.y = tTim.cy;
		tRect.w = tTim.cw;
		tRect.h = tTim.ch;
		LoadImage(&tRect, tTim.clut);	// Load CLUT into framebuffer
		DrawSync(0);
	}
	
	return tTim;
}

int LoadTIMFile(char* filename, GsIMAGE* image)
{
	int ntoc;
	u_char* buffer;

	if ((ntoc = PckSearchFile(&s_mainArchive, filename)) == -1)
	{
		return 0;
	}

	buffer = (u_char*)malloc(s_mainArchive.File[ntoc].Size + 2047);

	PckReadFileNum(&s_mainArchive, ntoc, (u_long*)buffer, s_mainArchive.File[ntoc].Size);
	CdReadSync(0, 0);

	if (image != 0)
	{
		*image = LoadTIM((u_long*)buffer);
	}
	else
	{
		LoadTIM((u_long*)buffer);
	}
	
	free(buffer);

	return 1;
}

void SetSpritePosition(GsSPRITE* sprite, GsIMAGE* timParams, short x, short y)
{
	// Set texture X coordinate
	switch (timParams->pmode & 3) 
	{
		case 0: // 4-bit
			sprite->u = ((timParams->px & 0x3f) * 4) + (x % 256);
			break;
		case 1: // 8-bit
			sprite->u		= ((timParams->px & 0x3f) * 2) + (x % 256);
			sprite->tpage	= GetTPage(timParams->pmode & 3, 0, timParams->px + (128 * (x / 256)), timParams->py + (256 * (y / 256)));
			break;
		default: // 16-bit
			sprite->u = (timParams->px & 0x3f) + (x % 256);
	};

	// Set texture Y coordinate
	sprite->v = (timParams->py & 0xff) + (y % 256);
}

volatile int fps;
volatile int fps_counter;
volatile int fps_measure;

void vsync_cb()
{
    fps_counter++;
    if( fps_counter >= 50 )
	{
        fps = fps_measure;
        fps_measure = 0;
        fps_counter = 0;
    }
}

void InitGraphics()
{
	int i;

	/* Reset all callbacks to 0. */
	ResetCallback();

	/* Initialize graphics system */
	ResetGraph(0);
	SetGraphDebug(0);

	#define SCREEN_WIDTH 320
	#define SCREEN_HEIGHT 240

	/* Set video mode */
	SetVideoMode(MODE_PAL);

	/* Define display buffer and back buffer area in VRAM */
	GsDISPENV.screen.y = 8;
	GsDISPENV.screen.h = 256;
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER | GsOFSGPU, 1, 0);
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);

	/* Initialize OT */
	for(i = 0; i < 2; i++) 
	{
		WorldOT[i].length = OT_LENGTH;
		WorldOT[i].org = OTTags[i];
		GsClearOt(0,0,&WorldOT[i]);
	}
	
	s_clearColor.red = 0;
	s_clearColor.green = 0;
	s_clearColor.blue = 0;

	VSyncCallback(vsync_cb);
}

void DrawSprite(GsSPRITE* sprite)
{
	GsSortFastSprite(sprite, &WorldOT[s_activeBuff], 0);
}

GsSPRITE CreateSprite(GsIMAGE TimParams, int u, int v, int w, int h, int mx, int my)
{
	GsSPRITE tSprite;
	SetSpritePosition(&tSprite, &TimParams, u, v);

	// Set size of sprite
	tSprite.w = w;
	tSprite.h = h;
	
	// Set color mode of sprite
	tSprite.attribute	= (TimParams.pmode & 3) << 24;
	
	// CLUT coords
	tSprite.cx 			= TimParams.cx;
	tSprite.cy 			= TimParams.cy;
	
	// Set default position, color intensity, and scale/rotation values of sprite
	tSprite.x=tSprite.y				= 0;
	tSprite.mx=tSprite.my			= 0;
	tSprite.r=tSprite.g=tSprite.b	= 128;
	tSprite.scalex=tSprite.scaley	= ONE;
	tSprite.rotate					= 0;
	
	return tSprite;
}

void EngineInit(char* dataImage)
{
	InitGraphics();

	CdInit();
	CdSetDebug(0);

	/* Load game archive */
	if (!PckGetToc(dataImage, &s_mainArchive))
	{
		ErrorMessage("%s not found or damaged!", dataImage);
	}

	/* Load font TIM */
	if (!LoadTIMFile("FONT.TIM", &s_fontImage))
	{
		ErrorMessage("FONT.TIM not found in game archive!");
	}

	// Set color mode of sprite
	s_fontSprite.attribute	= (s_fontImage.pmode & 3) << 24;

	// CLUT coords
	s_fontSprite.cx 			= s_fontImage.cx;
	s_fontSprite.cy 			= s_fontImage.cy;

	// Set default position, color intensity, and scale/rotation values of sprite
	s_fontSprite.x=s_fontSprite.y			= 0;
	s_fontSprite.mx=s_fontSprite.my			= 0;
	s_fontSprite.r=s_fontSprite.g=s_fontSprite.b	= 128;
	s_fontSprite.scalex=s_fontSprite.scaley	= ONE;
	s_fontSprite.rotate					= 0;
}

TextPosition DrawTextColored(char* text, short x, short y, u_char r, u_char g, u_char b)
{
	TextPosition position;
	position.x = x;
	position.y = y;

	s_fontSprite.attribute |= (1 << 30);
	s_fontSprite.r = r;
	s_fontSprite.g = g;
	s_fontSprite.b = b;

	while(*text != 0)
	{
		if (!CHAR_VALID(*text))
		{
			text++;
			continue;
		}

		SetSpritePosition(&s_fontSprite, &s_fontImage, CHAR_X(*text), CHAR_Y(*text));
		s_fontSprite.x = position.x + CHAR_XOFF(*text);
		s_fontSprite.y = position.y + CHAR_YOFF(*text);
		s_fontSprite.w = CHAR_W(*text);
		s_fontSprite.h = CHAR_H(*text);

		GsSortFastSprite(&s_fontSprite, &WorldOT[s_activeBuff], 0);
		
		position.x += CHAR_ADV(*text) + 2;
		if (position.x >= 320)
		{
			break;
		}
		
		text++;
	}

	return position;
}

TextPosition DrawText(char* text, short x, short y)
{
	return DrawTextColored(text, x, y, 128, 128, 128);
}

TextPosition DrawFormat(short x, short y, char* text, ...)
{
	char buffer[128];

	va_list list;

	va_start(list, text);
	sprintf(buffer, text, list);
	va_end(list);

	return DrawTextColored(buffer, x, y, 128, 128, 128);
}

void ErrorMessage(char* format, ...)
{
	char buffer[512];
	va_list list;

	va_start( list, format );
    sprintf(buffer, format, list);
    va_end( list );

	/* Initialize system font for easy text rendering */
	FntLoad(960, 0); /* x, y in video memory buffer */
	SetDumpFnt(FntOpen(0, 0, 320, 240, 0, 600)); /* screen x, y, w, h, clearBg, maxChars*/

	SwapTo2D();

	/* Ensure rendering is enabled */
	SetDispMask(1);

	while(1) 
	{
		BeginFrame();
		FntPrint("\n\n ERROR:\n %s\n\n SYSTEM HALTED\n", buffer);
		FntFlush(-1);
		EndFrame();
	}
}

void BeginFrame()
{
	s_activeBuff = GsGetActiveBuff();
	GsSetWorkBase((PACKET *)GpuPacketArea[s_activeBuff]);
	GsClearOt(0, 0, &WorldOT[s_activeBuff]);
}

void Clear()
{
}

void EndFrame()
{
	int vsyncsSinceGameLaunch;

	DrawSync(0);

	VSync(0);
	fps_measure++;

	GsSwapDispBuff();
	GsSortClear(s_clearColor.red, s_clearColor.green, s_clearColor.blue, &WorldOT[s_activeBuff]);
	GsDrawOt(&WorldOT[s_activeBuff]);
}

u_long* LoadFile(char* filename, int* size)
{
	int ntoc;
	u_char* buffer;

	if ((ntoc = PckSearchFile(&s_mainArchive, filename)) == -1)
	{
		return 0;
	}

	buffer = (u_char*)malloc(s_mainArchive.File[ntoc].Size + 2047);
	if (buffer == 0)
	{
		return 0;
	}

	PckReadFileNum(&s_mainArchive, ntoc, (u_long*)buffer, s_mainArchive.File[ntoc].Size);
	CdReadSync(0, 0);

	if (size != 0)
	{
		*size = s_mainArchive.File[ntoc].Size;
	}

	return (u_long*)buffer;
}
