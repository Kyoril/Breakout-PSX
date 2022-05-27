REM This script generates the TMD files out of the RSD files in this directory.
REM It is not automatically executed from the BUILD.BAT file, as I don't expect
REM to change the game data files in the same interval as the build process.
REM However, after cooking, you will need to execute BUILD.BAT again in order
REM to generate a new PCK file with the generated TMD files.

if exist BALL.TMD DEL BALL.TMD
if exist LVBORDER.TMD DEL LVBORDER.TMD
if exist LVFLOOR.TMD DEL LVFLOOR.TMD
if exist PADDLE.TMD DEL PADDLE.TMD
if exist BLOCK01.TMD DEL BLOCK01.TMD
if exist BLOCK02.TMD DEL BLOCK02.TMD
if exist BLOCK03.TMD DEL BLOCK03.TMD
if exist BLOCK04.TMD DEL BLOCK04.TMD

rsdlink -s 32.0 -o BALL.TMD BALL.RSD
rsdlink -s 32.0 -o PADDLE.TMD PADDLE.RSD
rsdlink -s 32.0 -o LVBORDER.TMD LVBORDER.RSD
rsdlink -s 32.0 -o LVFLOOR.TMD LVFLOOR.RSD
rsdlink -s 32.0 -o BLOCK01.TMD BLOCK01.RSD
rsdlink -s 32.0 -o BLOCK02.TMD BLOCK02.RSD
rsdlink -s 32.0 -o BLOCK03.TMD BLOCK03.RSD
rsdlink -s 32.0 -o BLOCK04.TMD BLOCK04.RSD

if exist ..\BALL.TMD DEL ..\BALL.TMD
if exist ..\LVBORDER.TMD DEL ..\LVBORDER.TMD
if exist ..\LVFLOOR.TMD DEL ..\LVFLOOR.TMD
if exist ..\PADDLE.TMD DEL ..\PADDLE.TMD
if exist ..\BLOCK01.TMD DEL ..\BLOCK01.TMD
if exist ..\BLOCK02.TMD DEL ..\BLOCK02.TMD
if exist ..\BLOCK03.TMD DEL ..\BLOCK03.TMD
if exist ..\BLOCK04.TMD DEL ..\BLOCK04.TMD

tmdsort -o ..\BALL.TMD BALL.TMD
tmdsort -o ..\PADDLE.TMD PADDLE.TMD
tmdsort -o ..\LVBORDER.TMD LVBORDER.TMD
tmdsort -o ..\LVFLOOR.TMD LVFLOOR.TMD
tmdsort -o ..\BLOCK01.TMD BLOCK01.TMD
tmdsort -o ..\BLOCK02.TMD BLOCK02.TMD
tmdsort -o ..\BLOCK03.TMD BLOCK03.TMD
tmdsort -o ..\BLOCK04.TMD BLOCK04.TMD

pause