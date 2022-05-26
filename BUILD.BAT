@ECHO OFF
REM Building game code...

SET rootdir=%~dp0
if exist DISC\BREAKOUT.SYM del DISC\BREAKOUT.SYM
if exist DISC\BREAKOUT.EXE del DISC\BREAKOUT.EXE

cd SRC
psymake || GOTO build_error
move /Y %rootdir:~0,-1%\SRC\BREAKOUT.EXE ..\DISC\
move /Y %rootdir:~0,-1%\SRC\BREAKOUT.SYM ..\DISC\

cd ..
REM Packing game data...
if exist .\DISC\BREAKOUT.PCK del .\DISC\BREAKOUT.PCK
.\TOOLS\MPACK /Q .\DATA\BREAKOUT.TXT
move /Y %rootdir:~0,-1%\BREAKOUT.PCK DISC\

cd DISC
REM Building CD image, this may take a few moments...
BUILDCD -l -iBREAKOUT.IMG BREAKOUT.CTI
stripiso s 2352 BREAKOUT.IMG BREAKOUT.ISO

REM Cleanup
if exist BREAKOUT.IMG del BREAKOUT.IMG
if exist CDW900E.TOC del CDW900E.TOC
if exist PCD600.TOC del PCD600.TOC
if exist QSHEET.TOC del QSHEET.TOC

REM Licensing CD image....
PSXLICENSE /eu /i BREAKOUT.ISO

cd ..
move /Y %rootdir:~0,-1%\DISC\BREAKOUT.ISO %rootdir:~0,-1%\
GOTO finish

:build_error
cd %rootdir:~0,-1%
REM ECHO There was a build error!
PAUSE

:finish

