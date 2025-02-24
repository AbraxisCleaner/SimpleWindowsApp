#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>

#include <dxgi.h>

#include "win_resource.h"

extern int _fltused = 0;

struct globals_t
{
    HACCEL Accelerators;
    HFONT Font;
    HWND Hwnd;
    RECT RcFrame;
    RECT RcClient;
    HMENU Menu;
    int8_t HasUserQuit;
    int8_t IsHwndFullscreen;
};
struct globals_t *_Globals;

LRESULT EditorWndProc( HWND, UINT, WPARAM, LPARAM ); 

void WinMainCRTStartup( void )
{
    _Globals = (struct globals_t *)malloc(sizeof(struct globals_t));
    memset(_Globals, 0, sizeof(struct globals_t));
    
    _Globals->Font = CreateFontA(50, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Consolas");

    //  CREATE WINDOW
    WNDCLASSEXA Wc = {0};
    Wc.cbSize = sizeof(Wc);
    Wc.hInstance = GetModuleHandleA(NULL);
    Wc.hCursor = LoadCursorW(Wc.hInstance, IDC_ARROW);
    Wc.lpszClassName = "MyWndClass";
    Wc.lpfnWndProc = EditorWndProc;
    RegisterClassExA(&Wc);

    MONITORINFO Monitor = { sizeof(Monitor) };
    POINT MonitorPt = {0};
    GetMonitorInfoA(MonitorFromPoint(MonitorPt, 1), &Monitor);

    _Globals->RcFrame.right = 1600;
    _Globals->RcFrame.bottom = 1200;
    AdjustWindowRect(&_Globals->RcFrame, WS_TILEDWINDOW, FALSE);

    int St = WS_TILEDWINDOW;
    int Cw = (_Globals->RcFrame.right - _Globals->RcFrame.left);
    int Ch = (_Globals->RcFrame.bottom - _Globals->RcFrame.top);
    int Px = (((Monitor.rcWork.right - Monitor.rcWork.left) / 2) - (Cw / 2));
    int Py = (((Monitor.rcWork.bottom - Monitor.rcWork.top) / 2) - (Ch / 2));

    _Globals->Hwnd = CreateWindowExA(0, Wc.lpszClassName, "MyWindow", St, Px, Py, Cw, Ch, NULL, NULL, Wc.hInstance, NULL);
    GetWindowRect(_Globals->Hwnd, &_Globals->RcFrame);
    GetClientRect(_Globals->Hwnd, &_Globals->RcClient);

    // Disable automatic handling of alt-enter so we can handle it ourselves.
    IDXGIFactory *DxgiFactory;
    CreateDXGIFactory(&IID_IDXGIFactory, &DxgiFactory);
    DxgiFactory->lpVtbl->MakeWindowAssociation(DxgiFactory, _Globals->Hwnd, DXGI_MWA_NO_ALT_ENTER);
    DxgiFactory->lpVtbl->Release(DxgiFactory);

    // Load accelerator table and menu.
    _Globals->Accelerators = LoadAcceleratorsA(Wc.hInstance, MAKEINTRESOURCEA(IDR_ACCELTABLE));
    _Globals->Menu = LoadMenuA(Wc.hInstance, MAKEINTRESOURCEA(IDR_MENU));
    SetMenu(_Globals->Hwnd, _Globals->Menu);

    /* -- MAIN LOOP -- */
    ShowWindow(_Globals->Hwnd, SW_SHOW);
    MSG Msg;
    while (!_Globals->HasUserQuit)
    {
        while (PeekMessageA(&Msg, _Globals->Hwnd, 0, 0, PM_REMOVE))
        {
            if (!TranslateAcceleratorA(_Globals->Hwnd, _Globals->Accelerators, &Msg))
            {
                TranslateMessage(&Msg);
                DispatchMessageA(&Msg);
            }
        }
    }

    // Shutdown
    DestroyWindow(_Globals->Hwnd);

    DeleteObject(_Globals->Font);
    //FreeConsole();
    free(_Globals);
    ExitProcess(0);
}

LRESULT EditorWndProc( HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam )
{
    if (_Globals->Hwnd)
    {
        switch (umsg)
        {
            case WM_CLOSE:
            {
                _Globals->HasUserQuit = TRUE;
                ShowWindow(_Globals->Hwnd, SW_HIDE);
            }
            return 0;

            case WM_SIZE:
            {
                GetWindowRect(hwnd, &_Globals->RcFrame);
                GetClientRect(hwnd, &_Globals->RcClient);
                InvalidateRect(hwnd, &_Globals->RcClient, FALSE);
            }
            break;

            case WM_SIZING:
            {
                RECT *Rect = (RECT *)lparam;
                if ((Rect->right - Rect->left) < 500)
                    Rect->right = (Rect->left + 500);
                if ((Rect->bottom - Rect->top) < 500)
                    Rect->bottom = (Rect->top + 500);
            }
            break;

            case WM_PAINT:
            {
                PAINTSTRUCT Ps;
                HDC Dc = BeginPaint(hwnd, &Ps);
                HBRUSH Hbr;
                
                Hbr = CreateSolidBrush((COLORREF)0x0000FFFF);
                FillRect(Dc, &Ps.rcPaint, Hbr);
                
                SelectObject(Dc, _Globals->Font);
                SetBkColor(Dc, (COLORREF)0x0000FFFF);
                SetTextColor(Dc, (COLORREF)0x00FF00FF);
                DrawTextA(Dc, "Hello, World!", -1, &Ps.rcPaint, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
                DeleteObject(Hbr);

                EndPaint(hwnd, &Ps);
            }
            break;

            case WM_COMMAND:
            {
                int Cmd = LOWORD(wparam);

                switch (Cmd)
                {
                    case ID_ACCEL_ALTENTER:
                    {
                        MONITORINFO Monitor = { sizeof(Monitor) };
                        POINT MonitorPt = { _Globals->RcFrame.left, _Globals->RcFrame.top };
                        GetMonitorInfoA(MonitorFromPoint(MonitorPt, 1), &Monitor);

                        RECT NewRect = {0};
                        int st, cw, ch, px, py;

                        if (!_Globals->IsHwndFullscreen)
                        {
                            st = WS_POPUP;
                            px = Monitor.rcMonitor.left;
                            py = Monitor.rcMonitor.top;
                            cw = (Monitor.rcMonitor.right - Monitor.rcMonitor.left);
                            ch = (Monitor.rcMonitor.bottom - Monitor.rcMonitor.top);
                        }
                        else
                        {
                            st = WS_TILEDWINDOW; 

                            NewRect.right = 1600;
                            NewRect.bottom = 1200;
                            AdjustWindowRect(&NewRect, WS_TILEDWINDOW, FALSE);

                            cw = (NewRect.right - NewRect.left);
                            ch = (NewRect.bottom - NewRect.top);
                            px = (((Monitor.rcWork.right - Monitor.rcWork.left) / 2) - (cw / 2));
                            py = (((Monitor.rcWork.bottom - Monitor.rcWork.top) / 2) - (ch / 2));
                        }

                        SetWindowLongA(hwnd, GWL_STYLE, st);
                        SetWindowPos(hwnd, NULL, px, py, cw, ch, SWP_SHOWWINDOW);
                        _Globals->IsHwndFullscreen = !_Globals->IsHwndFullscreen;
                    }
                    break;

                    case ID_FILEMENU_EXIT:
                    {
                        _Globals->HasUserQuit = TRUE;
                        ShowWindow(hwnd, SW_HIDE);
                    }
                    break;
                }
            }
            break;
        }
    }
    return DefWindowProcA(hwnd, umsg, wparam, lparam);
}