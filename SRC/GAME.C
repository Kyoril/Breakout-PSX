/*
 * This file handles the main state of the game.
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
	VECTOR	pos;
	VECTOR	lookAt;
	GsRVIEW2 view;
	GsCOORDINATE2 coord2;
} Camera = {0};

// Object handler
#define MAX_OBJECTS 4
GsDOBJ2	Object[MAX_OBJECTS]={0};
int		ObjectCount=0;

/* Struct which contains the state of a single ball. */
typedef struct {
	/* If set to 1, the ball is enabled (active in the game). */
	u_char enabled;
	/* If set to 1, the ball is currently grabbed by the paddle. */
	u_char grabbed;
	/* The absolute position of the ball if it isn't grabbed. */
	VECTOR pos;
	/* The position relative to the paddle position if the ball is grabbed. */
	VECTOR grabbedPos;
	/* The ball's velocity. */
	VECTOR vel;
} Ball;

/* The maximum amount of balls that can be active in the game at the same time. */
#define MAX_BALLS 16

/* Ball instances of the game. */
static Ball s_balls[MAX_BALLS] = {0};

/* Struct for the paddle. */
static struct {
	VECTOR pos;
	VECTOR vel;
	SVECTOR rot;
} s_paddle={0};

/* 
 * Tries to add a new ball to the game. On success, the ball index is returned.
 * If there is no more room for a new ball, -1 is returned.
 */
int InitBall(u_char grabbed, VECTOR* position)
{
	int i;

	for (i = 0; i < MAX_BALLS; ++i)
	{
		if (s_balls[i].enabled)
		{
			continue;
		}

		s_balls[i].enabled = 1;
		s_balls[i].grabbed = grabbed;
		if (position != 0)
		{
			s_balls[i].grabbedPos.vx = position->vx;
			s_balls[i].grabbedPos.vy = position->vy;
			s_balls[i].grabbedPos.vz = position->vz;
			s_balls[i].pos.vx = position->vx;
			s_balls[i].pos.vy = position->vy;
			s_balls[i].pos.vz = position->vz;
		}
		else
		{
			setVector(&s_balls[i].grabbedPos, 0, 0, 0);
			setVector(&s_balls[i].pos, s_paddle.pos.vx * ONE, s_paddle.pos.vy * ONE, s_paddle.pos.vz * ONE);
		}

		return i;
	}

	return -1;
}


/* Moves the paddle using the given controller packet for reading player input data from. */
void MovePaddle(ControllerPacket* controller)
{
	u_char axis;

	s_paddle.vel.vx = 0;

	if (!ControllerPacketIsValid(controller))
	{
		return;
	}

	/* Common controls */
	if (IsPadButtonPressed(controller, PAD_Left))
	{
		s_paddle.vel.vx = -10*ONE;
	}
	if (IsPadButtonPressed(controller, PAD_Right))
	{
		s_paddle.vel.vx = 10*ONE;
	}

	/* Analog controls */
	if (GetControllerType(controller) == CONTROLLER_TYPE_DUALSHOCK ||
		GetControllerType(controller) == CONTROLLER_TYPE_ANALOG)
	{
		axis = GetLeftAnalogStickX(controller);

		if (axis < 96)
		{
			s_paddle.vel.vx = -(((96 - axis) * 100) / 1050) * ONE;
		}
		else if(axis > 150)
		{
			s_paddle.vel.vx = ((axis - 150) * 100) / 1050 * ONE;
		}
	}

	s_paddle.pos.vx += s_paddle.vel.vx;
	
	if (s_paddle.pos.vx < -300*ONE) s_paddle.pos.vx = -300*ONE;
	if (s_paddle.pos.vx > 300*ONE) s_paddle.pos.vx = 300*ONE;
}

void crossProduct(SVECTOR *v0, SVECTOR *v1, VECTOR *out)
{
	out->vx = ((v0->vy*v1->vz)-(v0->vz*v1->vy))>>12;
	out->vy = ((v0->vz*v1->vx)-(v0->vx*v1->vz))>>12;
	out->vz = ((v0->vx*v1->vy)-(v0->vy*v1->vx))>>12;
}

void LookAt(VECTOR *eye, VECTOR *at, SVECTOR *up, MATRIX *mtx)
{
	VECTOR taxis;
	SVECTOR zaxis;
	SVECTOR xaxis;
	SVECTOR yaxis;
	VECTOR pos;
	VECTOR vec;

	setVector(&taxis, at->vx-eye->vx, at->vy-eye->vy, at->vz-eye->vz);
	VectorNormalS(&taxis, &zaxis);
    crossProduct(&zaxis, up, &taxis);
	VectorNormalS(&taxis, &xaxis);
	crossProduct(&zaxis, &xaxis, &taxis);
	VectorNormalS(&taxis, &yaxis);

	mtx->m[0][0] = xaxis.vx;	mtx->m[1][0] = yaxis.vx;	mtx->m[2][0] = zaxis.vx;
	mtx->m[0][1] = xaxis.vy;	mtx->m[1][1] = yaxis.vy;	mtx->m[2][1] = zaxis.vy;
	mtx->m[0][2] = xaxis.vz;	mtx->m[1][2] = yaxis.vz;	mtx->m[2][2] = zaxis.vz;

	pos.vx = -eye->vx;
	pos.vy = -eye->vy;
	pos.vz = -eye->vz;

	ApplyMatrixLV(mtx, &pos, &vec);
	TransMatrix(mtx, &vec);
}


/* Moves all balls that are currently active in the game. */
void MoveBalls()
{
	int i;
	int ballsAlive = 0;

	for (i = 0; i < MAX_BALLS; ++i)
	{
		if (!s_balls[i].enabled)
		{
			continue;
		}

		ballsAlive++;

		if (s_balls[i].grabbed)
		{
			s_balls[i].pos.vx = s_paddle.pos.vx + s_balls[i].grabbedPos.vx;
			s_balls[i].pos.vy = s_paddle.pos.vy + s_balls[i].grabbedPos.vy;
			s_balls[i].pos.vz = s_paddle.pos.vz + s_balls[i].grabbedPos.vz;
		}
		else 
		{
			addVector(&s_balls[i].pos, &s_balls[i].vel);

			/* Level collision */
			if (s_balls[i].pos.vx < -300*ONE)
			{
				s_balls[i].pos.vx = -290*ONE;
				s_balls[i].vel.vx *= -1;
			}

			if (s_balls[i].pos.vz > 150*ONE)
			{
				s_balls[i].pos.vz = 140*ONE;
				s_balls[i].vel.vz *= -1;
			}

			if (s_balls[i].pos.vx > 300*ONE)
			{
				s_balls[i].pos.vx = 290*ONE;
				s_balls[i].vel.vx *= -1;
			}

			/* Death zone */
			if (s_balls[i].pos.vz < -400*ONE)
			{
				s_balls[i].enabled = 0;
				ballsAlive--;
			}

			/* TODO: Block collision */

			/* Paddle collision */
			if ((s_balls[i].pos.vx >= s_paddle.pos.vx - 50*ONE) &&
				(s_balls[i].pos.vx <= s_paddle.pos.vx + 50*ONE))
			{
				/* Horizontally hits the paddle, check vertical collision */
				if ((s_balls[i].pos.vz <= s_paddle.pos.vz) &&
					(s_balls[i].pos.vz >= s_paddle.pos.vz - 20*ONE))
				{
					s_balls[i].pos.vz += 10*ONE;
					s_balls[i].vel.vz *= -1;

					if (abs(s_paddle.vel.vx) > 0)
					{
						//s_balls[i].vel.vx 
					}

					//VectorNormal(&s_balls[i].vel, &s_balls[i].vel);
				}
			}
		}
	}

	if (ballsAlive <= 0)
	{
		InitBall(1, 0);
	}
}

/* Tries to fire one ball which is currently grabbed by the paddle. */
void FireBall()
{
	int i;
	VECTOR vel;

	for (i = 0; i < MAX_BALLS; ++i)
	{
		if (!s_balls[i].enabled)
		{
			continue;
		}

		if (s_balls[i].grabbed)
		{
			s_balls[i].grabbed = 0;

			copyVector(&s_balls[i].pos, &s_paddle.pos);
			s_balls[i].pos.vz += 10 * ONE;

			setVector(&vel, -s_paddle.vel.vx, 0, 5 * ONE);
			VectorNormal(&vel, &vel);

			setVector(&s_balls[i].vel, vel.vx * 10, vel.vy * 10, vel.vz * 10);
			
			return;
		}
	}
}

/* Updates and sets the view matrix based on properties from the Camera struct. */
void CalculateCamera()
{
	// This function simply calculates the viewpoint matrix based on the camera coordinates...
	// It must be called on every frame before drawing any objects.
	VECTOR	vec;
	GsVIEW2 view;
	SVECTOR up;
	
	// Copy the camera (base) matrix for the viewpoint matrix
	view.view = Camera.coord2.coord;
	view.super = WORLD;
	
	setVector(&up, 0, -ONE, 0);
	setVector(&vec, Camera.lookAt.vx/ ONE, Camera.lookAt.vy / ONE, Camera.lookAt.vz / ONE);

	LookAt(&Camera.pos, &vec, &up, &view.view);

	// Set the viewpoint matrix to the GTE
	GsSetView2(&view);
}

/* Externals from the engine. TODO: Get rid of direct references in this file. */
extern int s_activeBuff;		/* The currently active buffer index to know which OT is active. */
extern GsOT WorldOT[2];			/* The order table for each frame buffer. */
extern volatile int fps;		/* The current FPS count. */

/* Adds a GsDOBJ2 object to the order table of the current frame with the given position and rotation. */
void PutObject(VECTOR pos, SVECTOR rot, GsDOBJ2 *obj)
{
	MATRIX lmtx,omtx;
	GsCOORDINATE2 coord;
	
	pos.vx /= ONE;
	pos.vy /= ONE;
	pos.vz /= ONE;

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

/* Constructs a GsDOBJ2 object from a loaded TMD file in memory. */
int LinkModel(u_long *tmd, GsDOBJ2 *obj) 
{
	u_long *dop;
	int i,NumObj;
	
	/* Copy pointer to TMD file so that the original pointer won't get destroyed */
	dop = tmd;
	
	/* Skip header and then remap the addresses inside the TMD file */
	dop++; GsMapModelingData(dop);
	
	/* Get object count */
	dop++; NumObj = *dop;

	/* Link object handler with the specified TMD */
	dop++;
	for(i=0; i<NumObj; i++)
	{
		GsLinkObject4((u_long)dop, &obj[i], i);
		obj[i].attribute = (1<<6);	/* Disables light source calculation */
	}
	
	/* Return the object count found inside the TMD*/
	return NumObj;
}



/* POinter to the loaded TMD file for the level floor model. */
static u_long* s_floorTMD = 0;
/* Pointer to the loaded TMD file for the level border model. */
static u_long* s_levelTMD = 0;
/* Pointer to the loaded TMD file for the paddle model. */
static u_long* s_paddleTMD = 0;
/* Pointer to the loaded TMD file for the ball model. */
static u_long* s_ballTMD = 0;

/* Loads all the resource files required by the game. */
static void LoadGameData()
{
	/* TODO: BACKGROUND loading while rendering a loading screen? */

	s_floorTMD = LoadFile("LVFLOOR.TMD", 0);
	if (!s_floorTMD)
	{
		ErrorMessage("Unable to load LVFLOOR.TMD file!");
	}

	s_levelTMD = LoadFile("LVBORDER.TMD", 0);
	if (!s_levelTMD)
	{
		ErrorMessage("Unable to load LVBORDER.TMD file!");
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

/* Initializes the game state. */
static void InitGsGame()
{
	int i;

	for(i = 0; i < MAX_BALLS; ++i)
	{
		s_balls[i].enabled = 0;
	}

	s_balls[i].enabled = 1;

	ObjectCount += LinkModel(s_levelTMD, &Object[0]);
	ObjectCount += LinkModel(s_floorTMD, &Object[1]);
	ObjectCount += LinkModel(s_paddleTMD, &Object[2]);
	ObjectCount += LinkModel(s_ballTMD, &Object[3]);

	Object[0].attribute = 0;
	Object[1].attribute = GsDIV2;
	Object[2].attribute = 0;
	Object[3].attribute = 0;

	setVector(&s_paddle.pos, 0, 0, -250*ONE);

	InitBall(1, 0);
}

/* Unloads loaded TMD files from DRAM. */
static void FreeGameData()
{
	if (s_floorTMD != 0)
	{
		free(s_floorTMD);
		s_floorTMD = 0;
	}

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

/* 
 * Handles the GS_GAME gamestate. This function is like a separate main function. 
 * When it ends, the game state is left. Return type is the new game state to enter
 * after GS_GAME.
 */
int HandleGsGame()
{
	int i;
	int activeBalls;
	VECTOR ball;

	ControllerPacket* controllerPacket;

	// Object coordinates
	VECTOR	plat_pos={0};
	SVECTOR	plat_rot={0};

	// Lighting coordinates
	GsF_LIGHT pslt;

	// Player coordinates
	struct {
		int x,xv;
		int y,yv;
		int z,zv;
		int pan,panv;
		int til,tilv;
	} Player = {0};

	LoadGameData();

	SwapTo3D();
	
	/* Initialize coordinates for the camera (it will be used as a base for future matrix calculations) */
	GsInitCoordinate2(WORLD, &Camera.coord2);
	
	/* Set ambient color (for lighting) */
	GsSetAmbient(ONE/10, ONE/8, ONE/4);
	
	/* Set default lighting mode */
	GsSetLightMode(0);

	InitGsGame();

	/* Default camera/player position */
	Player.x = ONE*0;
	Player.y = -ONE*236;
	Player.z = -ONE*400;
	Player.pan = 0;
	Player.til = -550;

	/* Setup light */
	pslt.r = 0xff; pslt.g = 0xff; pslt.b = 0xff;
	setVector(&pslt, 1*ONE, 5*ONE, -10*ONE);

	setVector(&Camera.pos, Player.x/ONE, Player.y/ONE, Player.z/ONE);

	while(1)
	{
		controllerPacket = GetControllerPacket(0);

		BeginFrame();

		MovePaddle(controllerPacket);
		MoveBalls();

		copyVector(&Camera.lookAt, &s_paddle.pos);
		
		activeBalls = 1;
		for (i = 0; i < MAX_BALLS; ++i)
		{
			if (s_balls[i].enabled && !s_balls[i].grabbed)
			{
				addVector(&Camera.lookAt, &s_paddle.pos);
				addVector(&Camera.lookAt, &s_balls[i].pos);
				activeBalls += 2;
			}
		}

		if (activeBalls > 1)
		{
			setVector(&Camera.lookAt, Camera.lookAt.vx / activeBalls,
				Camera.lookAt.vy / activeBalls,
				Camera.lookAt.vz / activeBalls);
		}
		
		// Calculate the camera and viewpoint matrix
		CalculateCamera();
		
		// Set the light source coordinates
		GsSetFlatLight(0, &pslt);
		
		PutObject(s_paddle.pos, s_paddle.rot, &Object[2]);	// Paddle

		// Balls
		for (i = 0; i < MAX_BALLS; ++i)
		{
			if (!s_balls[i].enabled)
			{
				continue;
			}

			PutObject(s_balls[i].pos, s_paddle.rot, &Object[3]);	// Ball
		}

		PutObject(plat_pos, plat_rot, &Object[0]);	// Level
		PutObject(plat_pos, plat_rot, &Object[1]);	// Level

		EndFrame();

		if (ControllerPacketIsValid(controllerPacket))
		{
			if (IsPadButtonPressed(controllerPacket, PAD_Cross))
			{
				FireBall();
			}

			if (IsPadButtonPressed(controllerPacket, PAD_Select))
			{
				break;
			}
		}
	}

	/* Disable rendering for now */
	SetDispMask(0);

	FreeGameData();

	SwapTo2D();

	return GS_TITLE;
}