@echo off
cls
pushd bin\obj

rc.exe /nologo /fo win_resource.res ..\..\src\win_resource.rc
cl.exe ..\..\src\win_main.c /D WIN32_LEAN_AND_MEAN /D UNICODE /D _UNICODE /D _DEBUG /std:c17 /Zi /GS- /Gs9999 /I ..\..\src\ /link /nologo /incremental:no /out:win.exe /nodefaultlib /subsystem:Windows win_resource.res kernel32.lib shell32.lib user32.lib gdi32.lib vcruntime.lib ucrt.lib dxguid.lib dxgi.lib
copy win.exe ..\.

popd
