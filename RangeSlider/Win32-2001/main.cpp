#include "precomp.h"
#include "resource.h"
#include "RSlider.h"

extern BOOL APIENTRY testDlgProc( HWND, UINT, WPARAM, LPARAM );
extern BOOL APIENTRY demoDlgProc( HWND, UINT, WPARAM, LPARAM );


int APIENTRY _tWinMain( HINSTANCE hinst, 
    HINSTANCE hinstPrev, LPTSTR pszCmdLine, int nShow )
{
    InitCommonControls();
    initRangeSlider( hinst );
//    DialogBox( hinst, MAKEINTRESOURCE( IDD_TEST ), 0, testDlgProc );
    DialogBox( hinst, MAKEINTRESOURCE( IDD_DEMO ), 0, demoDlgProc );
    return 0;
}

// end of file
