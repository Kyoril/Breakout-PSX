SRCS =INTRO.C TITLE.C GAME.C GAMEOVER.C BALL.C LEVEL.C PADDLE.C
OBJS =INTRO.OBJ TITLE.OBJ GAME.OBJ GAMEOVER.OBJ BALL.OBJ LEVEL.OBJ PADDLE.OBJ
	
main :
	ccpsx -O3 -Xo$80020000 BREAKOUT.c PCKLIB.C ENGINE.C TITLE.C GAME.C -oBREAKOUT.CPE,BREAKOUT.SYM
	cpe2x /ce BREAKOUT.CPE
	del BREAKOUT.CPE

