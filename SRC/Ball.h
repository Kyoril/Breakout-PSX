
#ifndef _BALL_H_
#define _BALL_H_


/* The maximum amount of balls that can be active in the game at the same time. */
#define MAX_BALLS 8

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
	
	u_char renderId;
} Ball;

#endif
