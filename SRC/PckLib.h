
#ifndef _PCKLIB_H_
#define _PCKLIB_H_

#include <sys/types.h>

typedef struct {
	char	Name[16];
	int		Size;
	int		Pos;
} PckENTRY;

typedef struct {
	char		ID[3];
	u_char		NumFiles;
	PckENTRY	File[85];
	int			BasePos;
} PckTOC;

// Prototypes
int		PckGetToc(char *filename, PckTOC *toc);
int		PckGetSubToc(PckTOC *SearchToc, char *FileName, PckTOC *Toc);
int		PckReadFile(PckTOC *Toc, char *FileName, u_long *Buff, int NumBytes);
void	PckReadFileNum(PckTOC *Toc, int Num, u_long *Buff, int NumBytes);
int		PckSearchFile(PckTOC *Toc, char *FileName);

#endif
