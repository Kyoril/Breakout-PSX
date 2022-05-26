/* PckLib was written by LameGuy64 (please visit https://psxdev.net for more details! */

#include "PckLib.h"

#include <libetc.h>
#include <libcd.h>

// To keep track of the last sector read because CdRead() won't work properly when called without a seek first
static int PckNextSector;

int PckGetToc(char *FileName, PckTOC *Toc) {
	
	/*	Description:
			
			*FileName	- Pointer to a file name of a PCK file.
			*Toc		- Pointer to a PckTOC variable to store the TOC data in.
			
			This function simply reads the first sector of a PCK file which contains the
			TOC data of the file which you'll need to read the files inside it.
			
			If your project uses multiple PCK files, it is best to keep the TOC information
			of each file in memory to speed up file access times a bit.
			
	*/
	
	u_char	Mode=CdlModeSpeed;
	CdlFILE File={0};
	
	// Search if the file exists and get its parameters if so
	if (CdSearchFile(&File, FileName) == 0) {
		return(0);
	}
	
	// Seek to the file so we can get the TOC
	CdControl(CdlSetloc, (u_char*)&File.pos, 0);
	CdRead(1, (u_long*)Toc, Mode);
	CdReadSync(0, 0);
	
	// Make sure that the file is a PCK file
	if (Toc->ID[0] != 'P') {
		if (Toc->ID[1] != 'C') {
			if (Toc->ID[2] != 'K') {
				return(0);
			}
			return(0);
		}
		return(0);
	}
	
	// Save the pack file's sector position for later
	Toc->BasePos = CdPosToInt(&File.pos);
	
	return(1);
	
}

int PckGetSubToc(PckTOC *SearchToc, char *FileName, PckTOC *Toc) {
	
	/*	Description:
		
		*SearchToc	- Pointer to a PckTOC variable to access the other PCK file.
		*FileName	- Pointer to a string of the name of a PCK file.
		*Toc		- Pointer to a PckTOC variable to store the TOC in.
		
		This works exactly the same as PckGetToc() but this one reads the TOC of a PCK file
		inside a PCK file.
		
	*/
	
	int		FileNum=0;
	CdlLOC	ReadPos;
	u_char	Mode=CdlModeSpeed;
	
	
	// Search if the file exists in the PCK file
	FileNum = PckSearchFile(SearchToc, FileName);
	if (FileNum == -1) {
		return(0);
	}
	
	// Seek to the pack file and get its TOC
	CdIntToPos(SearchToc->BasePos + SearchToc->File[FileNum].Pos, &ReadPos);
	CdControl(CdlSetloc, (u_char*)&ReadPos, 0);
	CdRead(1, (u_long*)Toc, Mode);
	CdReadSync(0, 0);
	
	// Make sure that the file is a PCK pack file
	if (Toc->ID[0] != 'P') {
		if (Toc->ID[1] != 'C') {
			if (Toc->ID[2] != 'K') {
				return(0);
			}
			return(0);
		}
		return(0);
	}
	
	// Save the pack file's sector position for later
	Toc->BasePos = CdPosToInt(&ReadPos);
	
	return(1);
	
}

int PckReadFile(PckTOC *Toc, char *FileName, u_long *Buff, int NumBytes) {

	/*	Description:
		
		*Toc		- Pointer to a PckTOC variable.
		*FileName	- Pointer to a file name to load.
		*Buff		- Pointer to where the read data will be stored.
		NumBytes	- Number of bytes to read (must be a multiple of 2048 bytes)
		
		This function is just a simplified version of PckReadFileNum(). For some additional
		details regarding the use of this function, check the PckReadFileNum() function.
		
		Take note that NumBytes will be quantized automatically to a multiple of 2048 bytes
		so if NumBytes is 1 for example, it'll read 2048 bytes instead.
		
	*/
	
	int Num=0;
	
	
	if (Toc != 0) {
		Num = PckSearchFile(Toc, FileName);
		if (Num == -1) return(0);
	}
	
	PckReadFileNum(Toc, Num, Buff, NumBytes);
	return(1);
	
}

void PckReadFileNum(PckTOC *Toc, int Num, u_long *Buff, int NumBytes) {
	
	/*	Description:
		
		*Toc		- Pointer to a PckTOC variable.
		Num			- Number of file to read.
		*Buff		- Pointer to where the read data will be loaded.
		NumBytes	- Number of bytes to read (must be a multiple of 2048 bytes)
		
		This function works very much like CdReadFile() except that it reads files inside
		a PCK file. If *Toc is zero, it'll continue reading from where the last read is
		located just like how CdReadFile() behaves if you give it a zero as the file name.
		
		Take note that NumBytes will be quantized automatically to a multiple of 2048 bytes
		so if NumBytes is 1 for example, it'll read 2048 bytes instead.
		
	*/
	
	u_char	Mode=CdlModeSpeed;
	int		NumSectors;
	int		DestSector;
	CdlLOC	Pos;
	
	
	// Avoid invalid file numbers
	if (Num < 0) return;
	
	// Calculate where the file to load is located
	if (Toc != 0) {
		DestSector = Toc->BasePos + Toc->File[Num].Pos; // Seek to start of file
	} else {
		DestSector = PckNextSector; // Seek to the next sector from the last read
	}
	
	// Seek to it!
	CdIntToPos(DestSector, &Pos);
	CdControl(CdlSetloc, (u_char*)&Pos, 0);
	
	// Convert bytes into sector multiples
	if ((NumBytes == 0) && (Toc != 0)) {
		NumSectors = (Toc->File[Num].Size+2047)/2048;
	} else {
		NumSectors = (NumBytes+2047)/2048;
		if (NumSectors == 0) return;
	}
	
	// Begin reading!
	CdRead(NumSectors, Buff, Mode);
	
	// Save last sector for proper sequential reading
	PckNextSector = DestSector + NumSectors;
	
}

int PckSearchFile(PckTOC *Toc, char *FileName) {
	
	/*	Description:
			
			*Toc		- Pointer to a PckTOC structure.
			*FileName	- Pointer to a file name to search.
			
			This function simply scans through a PckTOC structure for a matching file name
			specified by *FileName and returns its file number if found.
	
		Returns:
			>0 - Index of file found.
			-1 - File not found.
			
	*/
	
	int		i;
	char	TempName[16]={0};
	
	
	// Make the name of the file to search upper-cased
	for (i=0; FileName[i] != 0x00; i+=1) {
		TempName[i] = toupper(FileName[i]);
	}
	
	// Scan through the TOC for any matching file names
	for (i=0; i<Toc->NumFiles; i+=1) {
		if (strcmp(Toc->File[i].Name, TempName) == 0) {	// Found it!
			return(i);
		}
	}
	
	return(-1);
	
}