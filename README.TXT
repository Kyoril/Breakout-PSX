This folder contains the full source code and game data as well as tools and build
automation for the homebrew game BREAKOUT from KYORIL.


Prerequisites
*************

This game was built using the PSY-Q SDK, using runtime version 4.3. You need to have it installed and have
the PATH variables set as either environment variables or via AUTOEXEC.BAT in case you're using WIN9X or.

For a detailed guide on how to install and setup the PSY-Q SDK, please visit
http://www.psxdev.net/help/psyq_install.html.

For mastering the CD image (ISO), you will also need three tools from this article (BUILDCD, PSXLICENSE, STRIPISO). 
Please make sure to add them to your PATH environment variable:
http://www.psxdev.net/help/cdrom_mastering.html

NOTE: On Windows XP I found StripIso to not work. However, the tool "Revenge of StripISO" delivers an alternative
STRIPISO tool which works on my xp machine just fine: http://www.psxdev.net/forum/viewtopic.php?f=60&t=997


How to build
************

Simply execute BUILD.BAT in this directory and you're done. You should now have a BREAKOUT.ISO in this
directory which you can burn to DISC and execute on real (modchipped) hardware or load it using
your favorite emulator of choice!


How to edit source code
***********************`

In case you want to modify the source code, you will find a Visual Studio 2005 SLN file in SRC/.
However, please note that this is only due to the fact that I personally use VS2005 as my IDE for
code editing. It does not build directly from VS2005. Instead, the makefile is executed using
PSYMAKE, which is invoked from BUILD.BAT.

My usual workflow is to have the solution opened in VS2005, edit the code there, and execute BUILD.BAT
from time to time when I want to compile my code and test it. I usually have No$psx open in the background
and just hit Ctrl+R to reload the disc image immediately.

Feel free to modify this to your needs.


Folder structure
****************

========================================================================================================
SRC\		| Contains the source code files of the game.
DATA\		| Contains game data files as well as a recipe for how game data will be packed on disc.
TOOLS\		| Contains tools provided with the game.
DISC\		| Contains files which will be embedded into an ISO as well as generated files after 
		| executing BUILD.BAT, like BREAKOUT.EXE or BREAKOUT.PCK.
BUILD.BAT	| Build automation script. Executing it will build the source code and generate
		| BREAKOUT.ISO, which can be burnt directly for execution on real PS hardware.
BREAKOUT.ISO	| Generated when successfully running BUILD.BAT.
README.TXT	| This readme file.
========================================================================================================

