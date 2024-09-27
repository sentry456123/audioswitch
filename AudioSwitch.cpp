/* ***************************************************************************
 * Include Files
 * **************************************************************************/

#include "AudioSwitch.h"
#include "AudioPlaybackControl.h"


/* ***************************************************************************
 * Global Variables
 * **************************************************************************/

HHOOK _keyboard_hook = NULL;

HWND _window = NULL;

ATOM _windowClass = NULL;

bool _ctrlPressed = false;

/* ***************************************************************************
 * Functions
 * **************************************************************************/
 
#ifdef DEBUG

/* Attaches a console window to a regular Win32 app. */
bool CreateConsole( void )
{
    AllocConsole();

    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    return true;
}

#endif


/* Keyboard hook procedure. */
LRESULT CALLBACK KeyboardProc( int code, WPARAM wParam, LPARAM lParam )
{
    if ( code >= 0 )
    {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        bool keyDown = 0 == ( p->flags & LLKHF_UP );

        if ( p->vkCode == VK_RCONTROL || p->vkCode == VK_LCONTROL )
        {
            _ctrlPressed = keyDown;
        }

        if ( keyDown )
        {
            if ( _ctrlPressed && ( p->vkCode == VK_F12 ) )
            {
                NextAudioPlaybackDevice();
            }
        }
    }
        
    return CallNextHookEx( _keyboard_hook, code, wParam, lParam );
}


/* Window procedure function that processes messages sent to a window. 
 * Removes the hook when the window is destroyed. */
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_CLOSE:
            #ifdef DEBUG
            FreeConsole( );            
            #endif

            DestroyWindow( hwnd );
            UnhookWindowsHookEx( _keyboard_hook );                        
            break;

        case WM_DESTROY:
            PostQuitMessage( NULL );
            break;

        default:
            return DefWindowProc( hwnd, msg, wParam, lParam );
    }

    return 0;
}


/* The entry point.
 * Installs keyboard hook, creates an invisible window and waits for a message 
 * in a loop until window is closed (WM_CLOSE). */
int WINAPI WinMain (
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    #ifdef DEBUG
    if ( !CreateConsole() ) 
    {
        return EXIT_FAILURE;
    }
    #endif
       
    _keyboard_hook = SetWindowsHookEx( WH_KEYBOARD_LL, (HOOKPROC)KeyboardProc, GetModuleHandle( NULL ), NULL );
    if ( _keyboard_hook )
    {
        WNDCLASSEX wc{};
        MSG msg{};
        HICON icon = LoadIcon( hInstance, TEXT( "10026" ) );

        wc.cbSize        = sizeof wc;
        wc.lpszClassName = WINDOW_CLASS_NAME;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = hInstance;
        wc.hIconSm       = icon;
        wc.hIcon         = icon;

        _windowClass = RegisterClassEx( &wc );
        if ( _windowClass )
        {
            _window = CreateWindowEx( 
                NULL,                           /* dwExStyle    */
                WINDOW_CLASS_NAME,              /* lpClassName  */
                NULL,                           /* lpWindowName */
                NULL,                           /* dwStyle      */
                CW_USEDEFAULT,                  /* x            */
                CW_USEDEFAULT,                  /* y            */
                320,                            /* nWidth       */
                240,                            /* nHeight      */
                NULL,                           /* hWndParent   */
                NULL,                           /* hMenu        */
                hInstance,                      /* hInstance    */
                NULL                            /* lpParam      */
            );

            if ( _window )
            {
                while ( GetMessage( &msg, NULL, NULL, NULL ) > 0 )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                
                return EXIT_SUCCESS;
            }
        }
    }
    
    return EXIT_FAILURE;
}
