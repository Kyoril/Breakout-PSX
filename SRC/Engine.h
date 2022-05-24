
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "PckLib.h"

#include <libgs.h>

typedef struct
{
	short x, y;
} TextPosition;

/* Utility function to draw colored text on screen at a given position. */
void EngineInit(char* dataImage);
void ErrorMessage(char* format, ...);

void SwapTo3D();
void SwapTo2D();

void SetClearColor(u_char red, u_char green, u_char blue);
void BeginFrame();
void Clear();
void EndFrame();

GsSPRITE CreateSprite(GsIMAGE TimParams, int u, int v, int w, int h, int mx, int my);
void DrawSprite(GsSPRITE* sprite);
void SetSpritePosition(GsSPRITE* sprite, GsIMAGE* timParams, short x, short y);
TextPosition DrawTextColored(char* text, short x, short y, u_char r, u_char g, u_char b);
TextPosition DrawText(char* text, short x, short y);
TextPosition DrawFormat(short x, short y, char* text, ...);

u_long* LoadFile(char* filename, int* size);

int LoadTIMFile(char* filename, GsIMAGE* image);
GsIMAGE LoadTIM(u_long *tMemAddress);

#endif
