#include "precomp.h"
#include "getClientRect.h"
#include "fillRect.h"

void fillRect( HDC hdc, HWND hwnd, const RECT *prc, int color ) {

#if 1
    FillRect( hdc, prc, GetSysColorBrush( color ) );
    if ( COLOR_BTNSHADOW == color ) {
        return; //*** FUNCTION EXIT POINT
    }
#endif

#if 0
    HRGN hrgnClip = CreateRectRgnIndirect( prc );
    const int nSavedDC = SaveDC( hdc );
    SelectClipRgn( hdc, hrgnClip );

    UINT flags = BF_RECT | BF_SOFT;
    if ( !IsWindowEnabled( hwnd ) ) {
        flags |= BF_SOFT;
    }
    RECT rc = { 0 };
    const RECT rcClient = getClientRect( hwnd );
    const LONG range = getRange( hwnd );
    for ( int i = 0; i < range; i += 1 ) {
        rc = rcClient;
        rc.left  = pixFromSlider( hwnd, i );
        rc.right = pixFromSlider( hwnd, i + 1 );
        int pos = getStart( hwnd ) + i;
        //if ( pos < getLower( hwnd ) || getUpper( hwnd ) <= pos ) {
        //    DrawEdge( hdc, &rc, EDGE_RAISED, flags | BF_MIDDLE );
        //} else {
            DrawEdge( hdc, &rc, EDGE_RAISED, flags | BF_ADJUST );
            FillRect( hdc, &rc, GetSysColorBrush( color ) );
        //}
#if 0
        FillRect( hdc, &rc, GetSysColorBrush( COLOR_BTNFACE ) );
        rc.left  = pixFromSlider( hwnd, i + 1 ) -1;
        rc.right = rc.left + 1;
        FillRect( hdc, &rc, GetSysColorBrush( COLOR_BTNSHADOW ) );
#endif
    }

    RestoreDC( hdc, nSavedDC );
    DeleteRgn( hrgnClip );
#endif
}

