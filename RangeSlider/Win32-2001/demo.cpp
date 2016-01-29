#include "precomp.h"
#include "resource.h"
#include "RSlider.h"
#include "resize.h"
#include "resize2.h"


static void handleNotify( HWND hwnd, int id, NMRANGESLIDER *pInfo ) {

    char str[ 100 ] = { 0 };
    if ( IDC_PRICE == id ) {
        wsprintf( str, "$%d - $%d", pInfo->rsInfo._start, pInfo->rsInfo._end );
        SetDlgItemText( hwnd, IDC_PRICE_LABEL, str );
    } else if ( IDC_SHOESIZE == id ) {
        wsprintf( str, "%d - %d", pInfo->rsInfo._start, pInfo->rsInfo._end );
        SetDlgItemText( hwnd, IDC_SIZE_LABEL, str );
    }
}


static void handleCommand( HWND hwnd, int id, int cmd ) {

    switch ( id ) {
    case IDOK:
    case IDCANCEL:
        EndDialog( hwnd, id );
        break;

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

    makeResizable2( hwnd, IDC_EDIT );
//    setBehavior( hwnd, IDC_HORZ, right, both );

    setDlgItemSliderRange( hwnd, IDC_PRICE   , 0, 200,  0, 200, 0, 5 );
    setDlgItemSliderRange( hwnd, IDC_SHOESIZE, 4,  14,  4,  14, 0, 1 );
    setDlgItemSliderRange( hwnd, IDC_VERT    , 0, 100, 10,  50, 5, 0 );
    setDlgItemSliderRange( hwnd, IDC_HORZ    , 0, 100, 20,  80, 0, 1 );

    HWND hwndColor = GetDlgItem( hwnd, IDC_COLOR );
    ComboBox_AddString( hwndColor, "-- Any --" );
    ComboBox_AddString( hwndColor, "Black" );
    ComboBox_AddString( hwndColor, "Blue" );
    ComboBox_AddString( hwndColor, "Brown" );
    ComboBox_AddString( hwndColor, "Green" );
    ComboBox_AddString( hwndColor, "Red" );
    ComboBox_AddString( hwndColor, "White" );
    ComboBox_AddString( hwndColor, "Yellow" );

    HWND hwndProducer = GetDlgItem( hwnd, IDC_PRODUCER );
    ComboBox_AddString( hwndProducer, "-- Any --" );
    ComboBox_AddString( hwndProducer, "Ecco" );

    HWND hwndCountry = GetDlgItem( hwnd, IDC_COUNTRY );
    ComboBox_AddString( hwndCountry, "-- Any --" );
    ComboBox_AddString( hwndCountry, "Denmark" );
    ComboBox_AddString( hwndCountry, "EU" );
    ComboBox_AddString( hwndCountry, "India" );
    ComboBox_AddString( hwndCountry, "Italy" );
    ComboBox_AddString( hwndCountry, "Norway" );
    ComboBox_AddString( hwndCountry, "USA" );

    ComboBox_SetCurSel( hwndColor   , 0 );
    ComboBox_SetCurSel( hwndProducer, 0 );
    ComboBox_SetCurSel( hwndCountry , 0 );

    SetDlgItemText( hwnd, IDC_EDIT,
        "\r\nIn this space\r\n\r\n"
        "Imagine the results of some\r\n"
        "dynamic query\r\n\r\n"
        "Shoes, perhaps,\r\nmatching price\r\n"
        "or color\r\n"
        "or anything else\r\n"
        "..."
        );
}


BOOL APIENTRY demoDlgProc( 
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
