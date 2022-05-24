

#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include "Engine.h"

#include "Breakout.h"
#include "Control.h"
#include "Title.h"

/* force 2 megabytes of RAM */
u_long _ramsize   = 0x00200000;
/* force 16 kilobytes of stack */
u_long _stacksize = 0x00004000;

/* Stores controller data*/
ControllerPacket controllerPackets[MAX_CONTROLLER_COUNT];

int currentGameState = GS_TITLE;

ControllerPacket* GetControllerPacket(int port)
{
	return &controllerPackets[port];
}

/* Initializes the input system for handling user input. */
void InitInput()
{
	InitTAP(&controllerPackets[0], MAX_CONTROLLER_BYTES, &controllerPackets[1], MAX_CONTROLLER_BYTES);
	StartTAP();
}

/* Terminates the input system so that user input is no longer handled.*/
void TerminateInput()
{
	StopTAP();
}

/* Updates player input of the game. Returns FALSE if there is no (supported) controller connected to PORT 1. */
void EnsureSupportedControllerConnected()
{
	while(1)
	{
		/* If controller 1 is not connected, just return false and exit. */
		if (ControllerPacketIsValid(&controllerPackets[0]))
		{
			break;
		}

		/* Is this a controller type which is supported by us? */
		if (GetControllerType(&controllerPackets[0]) == CONTROLLER_TYPE_ANALOG ||
			GetControllerType(&controllerPackets[0]) == CONTROLLER_TYPE_DUALSHOCK ||
			GetControllerType(&controllerPackets[0]) == CONTROLLER_TYPE_PAD)
		{
			break;
		}

		BeginFrame();
		DrawText("No supported controller detected in", 0, 80);
		DrawText("port 1! Please connect a controller", 0, 96);
		DrawText("to continue.", 0, 112);
		EndFrame();
	}
}

int main()
{
	/* Initialize all the required engine systems*/
	EngineInit("\\BREAKOUT.PCK;1");

	/* Setup graphics subsystem*/
	InitGraphics();

	/* Setup input subsystem */
	InitInput();

	while(1)
	{
		int result;

		switch(currentGameState)
		{
		case GS_TITLE:
			result = HandleGsTitle();
			break;
		case GS_GAME:
			result = HandleGsGame();
			break;
		}

		if (result != -1)
		{
			currentGameState = result;
		}
	}
	
	TerminateInput();
	ResetGraph(3);
	StopCallback();

	return 0;
}
