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
#include "Ball.h"
#include "Level.h"

// Camera coordinates
struct {
	VECTOR	pos;
	VECTOR	lookAt;
	GsRVIEW2 view;
	GsCOORDINATE2 coord2;
} Camera = {0};

typedef struct {
	u_char type;
	u_char power;
	VECTOR pos;
	u_char renderId;
} Block;

// Object handler
#define MAX_OBJECTS 8
GsDOBJ2	Object[MAX_OBJECTS]={0};
int		ObjectCount=0;
u_char	ObjectSort[MAX_OBJECTS]={255};

#define MAX_BLOCKS 32
static Block s_blocks[MAX_BLOCKS];

u_char g_level;
long g_score;
short g_tries;

/* Ball instances of the game. */
static Ball s_balls[MAX_BALLS] = {0};

/* Struct for the paddle. */
static struct {
	VECTOR pos;
	VECTOR vel;
	SVECTOR rot;
} s_paddle={0};

#define BLOCK_ROW_HEIGHT(i) (150 - i * 34) - 16

void CreateBlockRow(const char* rowData, VECTOR rowPosition)
{
	int i;

	while(*rowData != 0 && *rowData != '\r' && *rowData != '\n')
	{
		if (*rowData != ' ')
		{
			for (i = 0; i < MAX_BLOCKS; ++i)
			{
				if (s_blocks[i].type != 0)
				{
					continue;
				}

				switch(*rowData)
				{
				case '1':
					s_blocks[i].type = 1;
					s_blocks[i].power = 1;
					break;
				case '2':
					s_blocks[i].type = 2;
					s_blocks[i].power = 2;
					break;
				case '3':
					s_blocks[i].type = 3;
					s_blocks[i].power = 4;
					break;
				case '4':
					s_blocks[i].type = 3;
					s_blocks[i].power = 3;
					break;
				default:
					ErrorMessage("Invalid block type discovered!");
					break;
				}
				
				copyVector(&s_blocks[i].pos, &rowPosition);
				break;
			}

			if (i == MAX_BLOCKS)
			{
				ErrorMessage("Maximum number of blocks reached!");
				break;
			}
		}

		rowPosition.vx += ONE * 64;
		rowData++;
	}
}

VECTOR makeVector(long x, long y, long z)
{
	VECTOR result;
	setVector(&result, x * ONE, y * ONE, z * ONE);
	return result;
}

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

void InitLevel(int level)
{
	int i;
	VECTOR position;

	for (i = 0; i < MAX_BLOCKS; ++i)
	{
		s_blocks[i].type = 0;
	}

	for (i = 0; i < MAX_BALLS; ++i)
	{
		s_balls[i].enabled = 0;
	}

	InitBall(1, 0);

	switch(level)
	{
	case 1:
		CreateBlockRow(" 1  1  1 ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(1)));
		CreateBlockRow("12 111 21", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow("12     21", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow(" 112 211 ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		break;

	case 2:
		CreateBlockRow("   222   ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow("  2 3 2  ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow(" 2 333 2 ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("111111111", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		break;

	case 3:
		CreateBlockRow("33  2  33", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow("   131   ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow("1 12121 1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("212 2 212", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		break;

	case 4:
		CreateBlockRow("111111111", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow("1   2   1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow("1 2 2 2 1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("1 2 3 2 1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		CreateBlockRow("333   333", makeVector(-280, 0, BLOCK_ROW_HEIGHT(6)));
		break;

	case 5:
		CreateBlockRow("1   3   1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow(" 1 343 1 ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow("  23 32  ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("  22122  ", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		break;

	case 6:
		CreateBlockRow("2  111  2", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow("2 14441 2", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow("1  333  1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("11     11", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		break;

	case 7:
		CreateBlockRow("1 2 2 2 1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(2)));
		CreateBlockRow("1       1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow("13 3 3 31", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("1       1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		CreateBlockRow("4 4 4 4 4", makeVector(-280, 0, BLOCK_ROW_HEIGHT(6)));
		break;

	case 8:
		CreateBlockRow("112222211", makeVector(-280, 0, BLOCK_ROW_HEIGHT(3)));
		CreateBlockRow("1   3   1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(4)));
		CreateBlockRow("1 2 4 2 1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(5)));
		CreateBlockRow("1 23332 1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(6)));
		CreateBlockRow("1  2 2  1", makeVector(-280, 0, BLOCK_ROW_HEIGHT(7)));
		break;

	default:
		ErrorMessage("Unsupported level %d!", level);
		break;
	}
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
	
	if (s_paddle.pos.vx - 32*ONE < -300*ONE) s_paddle.pos.vx = -300*ONE + 32*ONE;
	if (s_paddle.pos.vx + 32*ONE > 300*ONE) s_paddle.pos.vx = 300*ONE - 32*ONE;
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


#define MIN(a, b) ((a) < (b) ? a : b)

/* Moves all balls that are currently active in the game. */
int MoveBalls()
{
	int i, j;
	long distL, distR, distT, distB, minDist;
	int ballsAlive = 0;
	int blocksAlive = MAX_BLOCKS;

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
			for (j = 0; j < MAX_BLOCKS; ++j)
			{
				if (s_blocks[j].type == 0)
				{
					blocksAlive--;
					continue;
				}

				if (s_balls[i].pos.vx + (ONE*8) >= s_blocks[j].pos.vx - ONE*32 &&
					s_balls[i].pos.vx - (ONE*8) <= s_blocks[j].pos.vx + ONE*32 &&
					s_balls[i].pos.vz + (ONE*8) >= s_blocks[j].pos.vz - ONE*16 &&
					s_balls[i].pos.vz - (ONE*8) <= s_blocks[j].pos.vz + ONE*16)
				{
					s_blocks[j].power--;
					g_score += s_blocks[j].type;

					if (s_blocks[j].power == 0)
					{
						g_score += 100 * s_blocks[j].type;
						s_blocks[j].type = 0;
						blocksAlive--;
					}

					distL = abs(s_balls[i].pos.vx + ONE*8 - (s_blocks[j].pos.vx - ONE*32));
					distR = abs(s_balls[i].pos.vx - ONE*8 - (s_blocks[j].pos.vx + ONE*32));
					distT = abs(s_balls[i].pos.vz - ONE*8 - (s_blocks[j].pos.vz + ONE*16));
					distB = abs(s_balls[i].pos.vz + ONE*8 - (s_blocks[j].pos.vz - ONE*16));

					minDist = MIN(distL, MIN(distR, MIN(distT, distB)));

					if (minDist == distL || minDist == distR)
					{
						s_balls[i].vel.vx *= -1;

						if (minDist == distL)
						{
							s_balls[i].pos.vx -= ONE*3;
						}
						else
						{
							s_balls[i].pos.vx += ONE*3;
						}
					}
					else
					{
						s_balls[i].vel.vz *= -1;

						if (minDist == distT)
						{
							s_balls[i].pos.vz += ONE*3;
						}
						else
						{
							s_balls[i].pos.vz -= ONE*3;
						}
					}
				}
			}

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
					s_balls[i].vel.vx -= s_paddle.vel.vx / 3;
				}
			}
		}
	}

	if (blocksAlive == 0)
	{
		g_score += g_level * 10000;

		if (g_level == NUM_LEVEL)
		{
			g_tries++;
		}

		g_level = (g_level + 1) % NUM_LEVEL;
	}

	return ballsAlive;
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

			setVector(&vel, s_paddle.vel.vx / 3, 0, 4 * ONE / 2);
			VectorNormal(&vel, &vel);
			setVector(&s_balls[i].vel, vel.vx * 7, vel.vy * 7, vel.vz * 7);
			
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

#define NUM_BLOCK_TYPES 4
static u_long* s_blockTMD[NUM_BLOCK_TYPES] = {0};

/* Loads all the resource files required by the game. */
static void LoadGameData()
{
	int blockType;
	char buffer[16];

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

	for (blockType = 1; blockType <= NUM_BLOCK_TYPES; ++blockType)
	{
		sprintf(buffer, "BLOCK%02d.TMD", blockType);
		s_blockTMD[blockType-1] = LoadFile(buffer, 0);
		if (!s_blockTMD[blockType-1])
		{
			ErrorMessage("Unable to load %s file!", buffer);
		}
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

	for (i = 0; i < NUM_BLOCK_TYPES; ++i)
	{
		ObjectCount += LinkModel(s_blockTMD[i], &Object[4 + i]);
		Object[4 + i].attribute = 0;
	}

	Object[0].attribute = 0;
	Object[1].attribute = GsDIV2;
	Object[2].attribute = 0;
	Object[3].attribute = 0;

	setVector(&s_paddle.pos, 0, 0, -250*ONE);

	g_tries = 3;
	g_level = 1;
	g_score = 0;
}

/* Unloads loaded TMD files from DRAM. */
static void FreeGameData()
{
	int i;
	for (i = 0; i < NUM_BLOCK_TYPES; ++i)
	{
		if (s_blockTMD[i] != 0)
		{
			free(s_blockTMD[i]);
			s_blockTMD[i] = 0;
		}
	}

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
	int i, j;
	int activeBalls;
	int ballOffset;
	char buffer[64];
	VECTOR ball;
	u_char paused = 0;
	u_char startPressed = 0;
	int level = 0;

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
	} Player = {0};

	LoadGameData();

	SwapTo3D();
	
	/* Initialize coordinates for the camera (it will be used as a base for future matrix calculations) */
	GsInitCoordinate2(WORLD, &Camera.coord2);
	
	/* Set ambient color (for lighting) */
	GsSetAmbient(ONE/4, ONE/2, ONE/3);
	
	/* Set default lighting mode */
	GsSetLightMode(0);

	InitGsGame();

	/* Default camera/player position */
	Player.x = ONE*0;
	Player.y = -ONE*236;
	Player.z = -ONE*400;

	/* Setup light */
	pslt.r = 0xff; pslt.g = 0xff; pslt.b = 0xff;
	setVector(&pslt, 1*ONE, 5*ONE, -10*ONE);

	setVector(&Camera.lookAt, 0, 0, 0);

	pslt.vx = 0;
	pslt.vy = 1;
	pslt.vz = 3;

	while(1)
	{
		controllerPacket = GetControllerPacket(0);

		if (level != g_level)
		{
			level = g_level;
			InitLevel(level);
		}
		
		BeginFrame();

		if (paused)
		{
			DrawText("PAUSE", -40, -8);
		}
		else
		{
			MovePaddle(controllerPacket);
			if (MoveBalls() <= 0)
			{
				g_tries--;
				if (g_tries > 0)
				{
					InitBall(1, 0);
				}
			}

			copyVector(&Camera.pos, &s_paddle.pos);
			Camera.pos.vy -= 320 * ONE;
			Camera.pos.vz -= 160 * ONE;
			Camera.pos.vx /= ONE;
			Camera.pos.vy /= ONE;
			Camera.pos.vz /= ONE;
			
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
				activeBalls++;
				setVector(&Camera.lookAt, Camera.lookAt.vx / activeBalls,
					Camera.lookAt.vy / activeBalls,
					Camera.lookAt.vz / activeBalls);
			}
		}

		// Calculate the camera and viewpoint matrix
		CalculateCamera();
		
		// Set the light source coordinates
		GsSetFlatLight(0, &pslt);
		
		if (g_tries <= 0)
		{
			DrawText("GAME OVER", -30, -8);
			DrawText("Press SELECT to return", -92, -24);
		}
		else
		{
			sprintf(buffer, "Tries: %d", g_tries);
			DrawTextColored(buffer, -160, -120, 128, 32, 16);

			sprintf(buffer, "Level: %d", g_level);
			DrawTextColored(buffer, -160, -104, 32, 96, 32);

			sprintf(buffer, "Score: %d", g_score);
			DrawTextColored(buffer, -160, -88, 48, 64, 128);
		}

		PutObject(s_paddle.pos, s_paddle.rot, &Object[2]);	// Paddle

		/* TODO: Do proper sorting ffs T^T */

		/* Blocks */
		ballOffset = 0;
		for (i = MAX_BLOCKS - 1; i >= 0; --i)
		{
			if (s_blocks[i].type == 0)
			{
				continue;
			}

			PutObject(s_blocks[i].pos, s_paddle.rot, &Object[3 + s_blocks[i].type]); // Block
			
			/* Balls */
			for (j = ballOffset; j < MAX_BALLS; ++j)
			{
				if (!s_balls[j].enabled)
				{
					continue;
				}

				if (s_balls[j].pos.vz - ONE*8 < s_blocks[j].pos.vz + ONE*16)
				{
					continue;
				}

				PutObject(s_balls[j].pos, s_paddle.rot, &Object[3]);	// Ball
				ballOffset = i;
			}
		}

		for (j = ballOffset; j < MAX_BALLS; ++j)
		{
			if (!s_balls[j].enabled)
			{
				continue;
			}

			PutObject(s_balls[j].pos, s_paddle.rot, &Object[3]);	// Ball
		}

		PutObject(plat_pos, plat_rot, &Object[0]);	// Level
		PutObject(plat_pos, plat_rot, &Object[1]);	// Level
		

		EndFrame();

		if (ControllerPacketIsValid(controllerPacket))
		{
			if (g_tries > 0)
			{
				if (IsPadButtonPressed(controllerPacket, PAD_Cross))
				{
					FireBall();
				}

				if (IsPadButtonPressed(controllerPacket, PAD_Start))
				{
					if (!startPressed)
					{
						paused = !paused;
						startPressed = 1;
					}
				}
				else
				{
					startPressed = 0;
				}
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