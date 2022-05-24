
#ifndef _BREAKOUT_H_
#define _BREAKOUT_H_

#include "Control.h"

/* Display mode enum values*/
#define PAL MODE_PAL
#define NTSC MODE_NTSC


/* The current display mode to use */
#define DISPLAY_MODE PAL


/* SCREEN_WIDTH and SCREEN_HEIGHT macros for the given display mode. */
#define SCREEN_WIDTH 320
#if DISPLAY_MODE == PAL
#	define SCREEN_HEIGHT 256
#else
#	define SCREEN_HEIGHT 240
#endif


/* Enumerates available game states. */
enum GameStates
{
	/* The first game state */
	GS_TITLE,

	/* This  */
	GS_GAME
};

ControllerPacket* GetControllerPacket(int port);

#endif
