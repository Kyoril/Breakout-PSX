/*
 * Breakanoid
 * Copyright (C) 2022, Kyoril. All rights reserved.
 * ================================================
 */

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

#include "Title.h"
#include "Engine.h"
#include "Breakout.h"

// Camera coordinates
struct {
	int		x,y,z;
	int		pan,til,rol;
	VECTOR	pos;
	SVECTOR rot;
	GsRVIEW2 view;
	GsCOORDINATE2 coord2;
} Camera = {0};

// Object handler
#define MAX_OBJECTS 3
GsDOBJ2	Object[MAX_OBJECTS]={0};
int		ObjectCount=0;

void CalculateCamera()
{
	// This function simply calculates the viewpoint matrix based on the camera coordinates...
	// It must be called on every frame before drawing any objects.
	
	VECTOR	vec;
	GsVIEW2 view;
	
	// Copy the camera (base) matrix for the viewpoint matrix
	view.view = Camera.coord2.coord;
	view.super = WORLD;
	
	// I really can't explain how this works but I found it in one of the ZIMEN examples
	RotMatrix(&Camera.rot, &view.view);
	ApplyMatrixLV(&view.view, &Camera.pos, &vec);
	TransMatrix(&view.view, &vec);
	
	// Set the viewpoint matrix to the GTE
	GsSetView2(&view);
}

extern int s_activeBuff;
extern GsOT WorldOT[2];

void PutObject(VECTOR pos, SVECTOR rot, GsDOBJ2 *obj) {
	
	/*	This function draws (or sorts) a TMD model linked to a GsDOBJ2 structure... All
		matrix calculations are done automatically for simplified object placement.
		
		Parameters:
			pos 	- Object position.
			rot		- Object orientation.
			*obj	- Pointer to a GsDOBJ2 structure that is linked to a TMD model.
			
	*/
	
	MATRIX lmtx,omtx;
	GsCOORDINATE2 coord;
	
	// Copy the camera (base) matrix for the model
	coord = Camera.coord2;
	
	// Rotate and translate the matrix according to the specified coordinates
	RotMatrix(&rot, &omtx);
	TransMatrix(&omtx, &pos);
	CompMatrixLV(&Camera.coord2.coord, &omtx, &coord.coord);
	coord.flg = 0;
	
	// Apply coordinate matrix to the object
	obj->coord2 = &coord;
	
	// Calculate Local-World (for lighting) and Local-Screen (for projection) matrices and set both to the GTE
	GsGetLws(obj->coord2, &lmtx, &omtx);
	GsSetLightMatrix(&lmtx);
	GsSetLsMatrix(&omtx);
	
	// Sort the object!
	GsSortObject4(obj, &WorldOT[s_activeBuff], 14-1, getScratchAddr(0));
}

int LinkModel(u_long *tmd, GsDOBJ2 *obj) 
{
	/*	This function prepares the specified TMD model for drawing and then
		links it to a GsDOBJ2 structure so it can be drawn using GsSortObject4().
		
		By default, light source calculation is disabled but can be re-enabled by
		simply setting the attribute variable in your GsDOBJ2 structure to 0.
		
		Parameters:
			*tmd - Pointer to a TMD model file loaded in memory.
			*obj - Pointer to an empty GsDOBJ2 structure.
	
		Returns:
			Number of objects found inside the TMD file.
			
	*/
	
	u_long *dop;
	int i,NumObj;
	
	// Copy pointer to TMD file so that the original pointer won't get destroyed
	dop = tmd;
	
	// Skip header and then remap the addresses inside the TMD file
	dop++; GsMapModelingData(dop);
	
	// Get object count
	dop++; NumObj = *dop;

	// Link object handler with the specified TMD
	dop++;
	for(i=0; i<NumObj; i++) {
		GsLinkObject4((u_long)dop, &obj[i], i);
		obj[i].attribute = (1<<6);	// Disables light source calculation
	}
	
	// Return the object count found inside the TMD
	return(NumObj);
}

static u_long* s_levelTMD = 0;
static u_long* s_paddleTMD = 0;
static u_long* s_ballTMD = 0;

static void LoadGameData()
{
	s_levelTMD = LoadFile("LEVEL.TMD", 0);
	if (!s_levelTMD)
	{
		ErrorMessage("Unable to load LEVEL.TMD file!");
	}

	s_paddleTMD = LoadFile("PADDLE.TMD", 0);
	if (!s_paddleTMD)
	{
		ErrorMessage("Unable to load PADDLE.TMD file!");
	}

	s_ballTMD = LoadFile("BALL.TMD", 0);
	if (!s_ballTMD)
	{
		ErrorMessage("Unable to load BALL.TMD file!");
	}

	if (!LoadTIMFile("WOOD.TIM", 0))
	{
		ErrorMessage("Unable to load WOOD.TIM!");
	}

	if (!LoadTIMFile("BORDER.TIM", 0))
	{
		ErrorMessage("Unable to load BORDER.TIM!");
	}
}

static void InitGameData()
{
	ObjectCount += LinkModel(s_levelTMD, &Object[0]);
	ObjectCount += LinkModel(s_paddleTMD, &Object[1]);
	ObjectCount += LinkModel(s_ballTMD, &Object[2]);

	Object[0].attribute = 0;
	Object[1].attribute = 0;
	Object[2].attribute = 0;
}

static void FreeGameData()
{
	if (s_levelTMD != 0)
	{
		free(s_levelTMD);
		s_levelTMD = 0;
	}

	if (s_paddleTMD != 0)
	{
		free(s_paddleTMD);
		s_paddleTMD = 0;
	}

	if (s_ballTMD != 0)
	{
		free(s_ballTMD);
		s_ballTMD = 0;
	}
}

int HandleGsGame()
{
	char buffer[128];

	// Object coordinates
	VECTOR	paddlePos={0};
	VECTOR	plat_pos={0};
	SVECTOR	plat_rot={0};

	// Lighting coordinates
	GsF_LIGHT pslt[1];

	// Player coordinates
	struct {
		int x,xv;
		int y,yv;
		int z,zv;
		int pan,panv;
		int til,tilv;
	} Player = {0};

	ControllerPacket* controllerPacket;

	LoadGameData();

	SwapTo3D();
	
	// Initialize coordinates for the camera (it will be used as a base for future matrix calculations)
	GsInitCoordinate2(WORLD, &Camera.coord2);
	
	// Set ambient color (for lighting)
	GsSetAmbient(ONE/10, ONE/8, ONE/4);
	
	// Set default lighting mode
	GsSetLightMode(0);

	InitGameData();

	paddlePos.vx = 0;
	paddlePos.vy = 0;
	paddlePos.vz = -300;

	// Default camera/player position
	Player.x = ONE*0;
	Player.y = ONE*236;
	Player.z = ONE*376;
	Player.pan = 0;
	Player.til = -550;

	pslt[0].r =	0xff;	pslt[0].g = 0xff;	pslt[0].b = 0xff;

	while(1)
	{
		controllerPacket = GetControllerPacket(0);

		// Translate player coordinates for the camera
		Camera.pos.vx = Player.x/ONE;
		Camera.pos.vy = Player.y/ONE;
		Camera.pos.vz = Player.z/ONE;
		Camera.rot.vy = -Player.pan;
		Camera.rot.vx = -Player.til;

		// Calculate the camera and viewpoint matrix
		CalculateCamera();
		
		BeginFrame();

		sprintf(buffer, "PLR (%d, %d, %d)", Player.x / ONE, Player.y / ONE, Player.z / ONE);
		DrawText(buffer, -160, -120);

		// Set the light source coordinates
		pslt[0].vx = 1;
		pslt[0].vy = 5;
		pslt[0].vz = -10;
		GsSetFlatLight(0, &pslt[0]);
		

		PutObject(paddlePos, plat_rot, &Object[1]);	// Paddle
		PutObject(paddlePos, plat_rot, &Object[2]);	// Ball
		PutObject(plat_pos, plat_rot, &Object[0]);	// Level

		EndFrame();

		if (ControllerPacketIsValid(controllerPacket))
		{
			if (IsPadButtonPressed(controllerPacket, PAD_Up))
			{
				Player.y += ONE;
			}
			
			if (IsPadButtonPressed(controllerPacket, PAD_Down))
			{
				Player.y -= ONE;
			}
			
			if (IsPadButtonPressed(controllerPacket, PAD_Left))
			{
				Player.x -= ONE;
			}
			
			if (IsPadButtonPressed(controllerPacket, PAD_Right))
			{
				Player.x += ONE;
			}

			if (IsPadButtonPressed(controllerPacket, PAD_Triangle))
			{
				Player.z += ONE;
			}
			
			if (IsPadButtonPressed(controllerPacket, PAD_Cross))
			{
				Player.z -= ONE;
			}
			
			if (IsPadButtonPressed(controllerPacket, PAD_L1))
			{
				Player.til -= 1;
			}
			
			if (IsPadButtonPressed(controllerPacket, PAD_R1))
			{
				Player.til += 1;
			}

			if (IsPadButtonPressed(controllerPacket, PAD_Select))
			{
				break;
			}
		}
	}

	FreeGameData();

	SwapTo2D();

	return GS_TITLE;
}