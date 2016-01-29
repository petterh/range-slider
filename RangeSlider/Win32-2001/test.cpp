#include "precomp.h"
#include "resource.h"
#include "RSlider.h"
#include "resize.h"


static void setUpDownRange( HWND hwnd, int id ) {

    HWND hwndCtl = GetDlgItem( hwnd, id );
    SNDMSG( hwndCtl, UDM_SETRANGE32, 0x80000000, 0x7fffffff );
}


static BOOL busy = FALSE;
static void handleNotify( HWND hwnd, int id, NMRANGESLIDER *pInfo ) {

    if ( !busy && IDC_RANGE == id ) {
        busy = TRUE;
        SetDlgItemInt( hwnd, IDC_START, pInfo->rsInfo._start, TRUE );
        SetDlgItemInt( hwnd, IDC_END  , pInfo->rsInfo._end  , TRUE );
        busy = FALSE;
    }
}


static void handleCommand( HWND hwnd, int id, int cmd ) {

    switch ( id ) {
    case IDOK:
    case IDCANCEL:
        EndDialog( hwnd, id );
        break;
    case IDC_ENABLE:
        EnableWindow( GetDlgItem( hwnd, IDC_RANGE ),
            IsDlgButtonChecked( hwnd, IDC_ENABLE ) );
        break;
    case IDC_LOWER:
    case IDC_UPPER:
    case IDC_START:
    case IDC_END:
    case IDC_MINRANGE:
        if ( EN_CHANGE == cmd && !busy ) {
            setDlgItemSliderRange( hwnd, IDC_RANGE,
                GetDlgItemInt( hwnd, IDC_LOWER   , 0, TRUE ) ,
                GetDlgItemInt( hwnd, IDC_UPPER   , 0, TRUE ) ,
                GetDlgItemInt( hwnd, IDC_START   , 0, TRUE ) ,
                GetDlgItemInt( hwnd, IDC_END     , 0, TRUE ) ,
                GetDlgItemInt( hwnd, IDC_MINRANGE, 0, TRUE ) , 0 );
        }
    }
}


static void handleInitDialog( HWND hwnd ) {

    static CtlInfo g_aCtlInfo[] = {
        { IDC_RANGE , right, none   },
        { IDC_ENABLE, both , none   },
    };

    static Resizer g_resizer = {
        sizeof g_aCtlInfo / sizeof g_aCtlInfo[ 0 ], g_aCtlInfo,
    };

    makeResizable( hwnd, &g_resizer );

    setUpDownRange( hwnd, IDC_SPIN1 );
    setUpDownRange( hwnd, IDC_SPIN2 );
    setUpDownRange( hwnd, IDC_SPIN3 );
    setUpDownRange( hwnd, IDC_SPIN4 );
    setUpDownRange( hwnd, IDC_SPIN5 );
    SetDlgItemInt( hwnd, IDC_LOWER   , 10, TRUE );
    SetDlgItemInt( hwnd, IDC_UPPER   , 20, TRUE );
    SetDlgItemInt( hwnd, IDC_START   , 11, TRUE );
    SetDlgItemInt( hwnd, IDC_END     , 14, TRUE );
    SetDlgItemInt( hwnd, IDC_MINRANGE,  2, TRUE );
    CheckDlgButton( hwnd, IDC_ENABLE, 1 );
}

BOOL APIENTRY testDlgProc( 
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
    if ( WM_INITDIALOG == msg ) {
        handleInitDialog( hwnd );
        return TRUE;
    }
    if ( WM_NOTIFY == msg ) {
        handleNotify( hwnd, (int) wParam, (NMRANGESLIDER *) lParam );
        return TRUE;
    }
    if ( WM_COMMAND == msg ) {
        handleCommand( hwnd,
            GET_WM_COMMAND_ID ( wParam, lParam ),
            GET_WM_COMMAND_CMD( wParam, lParam ) );
        return TRUE;
    }

    return FALSE;
}

// end of file
