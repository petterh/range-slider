#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <stdlib.h>
#include "wdjsub.h"
#include "resize.h"

#ifndef HANDLE_WM_SIZING
/* BOOL onSizing( HWND hwnd, UINT fSize, LPRECT prc ) */
#define HANDLE_WM_SIZING( hwnd, wParam, lParam, fn ) \
    ((fn)((hwnd), (UINT)(wParam), (LPRECT)(lParam)))
#endif // HANDLE_WM_SIZING

#define freeResizer( p ) ( free( p->_pCtlInfo ), free( p ) )

static LRESULT CALLBACK resizeWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

static RECT getGripRect( HWND hwnd ) {
    RECT rc;
    GetClientRect( hwnd, &rc );
    rc.left = rc.right  - GetSystemMetrics( SM_CXVSCROLL );
    rc.top  = rc.bottom - GetSystemMetrics( SM_CXVSCROLL );
    return rc;
}

#define getResizer( hwnd ) \
    (Resizer *) wdjGetData( resizeWndProc, hwnd )
#define getResizer2( hwnd ) \
    (Resizer *) wdjGetData( resizeWndProc2, hwnd )

static RECT updateCtlRect( HWND hwnd, 
    HWND hwndCtrl, const CtlInfo *pItem, int dx, int dy )
{
    RECT rc;
    assert( IsWindow( hwndCtrl ) );
    GetWindowRect( hwndCtrl, &rc );
    MapWindowRect( HWND_DESKTOP, hwnd, &rc );

    // Moving the left side without the right makes no sense:
    assert( left != pItem->_nHorzAlignment );

    // Moving the top without the bottom makes no sense either:
    assert( top != pItem->_nVertAlignment );

    if ( left & pItem->_nHorzAlignment ) {
        rc.left += dx;
    }
    if ( right & pItem->_nHorzAlignment ) {
        rc.right += dx;
    }
    if ( top & pItem->_nVertAlignment ) {
        rc.top += dy;
    }
    if ( bottom & pItem->_nVertAlignment ) {
        rc.bottom += dy;
    }
    return rc;
}

static void moveChildren( HWND hwnd, int cx, int cy ) {
    int dx, dy, nItem;
    Resizer *pResizer = getResizer( hwnd );
    HDWP hdwp = BeginDeferWindowPos( pResizer->_nNumCtls );

    cx = max( cx, pResizer->_sizeMinClient.cx );
    cy = max( cy, pResizer->_sizeMinClient.cy );
    
    dx = cx - pResizer->_Prev.cx;
    dy = cy - pResizer->_Prev.cy;
    
    pResizer->_Prev.cx = cx;
    pResizer->_Prev.cy = cy;
    
    for ( nItem = 0; nItem < pResizer->_nNumCtls; ++nItem ) {
        const CtlInfo *pItem = &pResizer->_pCtlInfo[ nItem ];
        HWND hwndCtrl = GetDlgItem( hwnd, pItem->_nCtlId );
        RECT rc = updateCtlRect( hwnd, hwndCtrl, pItem, dx, dy );
        hdwp = DeferWindowPos( hdwp, hwndCtrl, 0, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER );
    }
    EndDeferWindowPos( hdwp );
}

static void onSize( HWND hwnd, UINT nType, int cx, int cy ) {
    wdjCallOldProc( resizeWndProc, 
        hwnd, WM_SIZE, (WPARAM) nType, MAKELPARAM( cx, cy ) );
    if ( SIZE_MINIMIZED != nType ) {
        RECT rc = getGripRect( hwnd );
        moveChildren( hwnd, cx, cy );
        InvalidateRect( hwnd, &rc, TRUE );
    }
}

static BOOL onSizing( HWND hwnd, UINT fSize, LPRECT prc ) {
    const BOOL bRet = wdjCallOldProc( resizeWndProc, 
        hwnd, WM_SIZING, (WPARAM) fSize, (LPARAM) prc );
    RECT rc = getGripRect( hwnd );
    InvalidateRect( hwnd, &rc, FALSE );
    return bRet;
}

static void onGetMinMaxInfo( HWND hwnd, MINMAXINFO FAR* pmmi ) {
    const Resizer *pResizer = getResizer( hwnd );
    pmmi->ptMinTrackSize.x = pResizer->_sizeMinTrack.cx;
    pmmi->ptMinTrackSize.y = pResizer->_sizeMinTrack.cy;
}

static BOOL onEraseBkgnd( HWND hwnd, HDC hdc ) {
    const BOOL bResult = (BOOL) wdjCallOldProc( 
        resizeWndProc, hwnd, WM_ERASEBKGND, (WPARAM) hdc, 0 );
    if ( !(WS_CHILD & GetWindowStyle( hwnd )) ) {
        RECT rc = getGripRect( hwnd );
        DrawFrameControl( hdc, &rc, 
            DFC_SCROLL, DFCS_SCROLLSIZEGRIP );
    }
    return bResult;
}

static UINT onNCHitTest( HWND hwnd, int x, int y ) {
    if ( !(WS_CHILD & GetWindowStyle( hwnd )) ) {
        int cx, cy;
        RECT rc = getGripRect( hwnd );
        MapWindowRect( hwnd, HWND_DESKTOP, &rc );
        cx = x - rc.left;
        cy = y - rc.top;
        if ( 0 < cx && 0 < cy ) {
            if ( rc.right - rc.left < cx + cy ) {
                return HTBOTTOMRIGHT;
            }
        }
    }
    return (UINT) wdjCallOldProc( 
        resizeWndProc, hwnd, WM_NCHITTEST, 0, MAKELPARAM( x, y ) );
}

static LRESULT CALLBACK resizeWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg ) {
    HANDLE_MSG( hwnd, WM_ERASEBKGND   , onEraseBkgnd    );
    HANDLE_MSG( hwnd, WM_NCHITTEST    , onNCHitTest     );
    HANDLE_MSG( hwnd, WM_SIZE         , onSize          );
    HANDLE_MSG( hwnd, WM_SIZING       , onSizing        );
    HANDLE_MSG( hwnd, WM_GETMINMAXINFO, onGetMinMaxInfo );
    }
    return wdjCallOldProc( 
        resizeWndProc, hwnd, msg, wParam, lParam );
}

BOOL makeResizable( HWND hwnd, Resizer *pResizer ) {
    RECT rc;
    GetWindowRect( hwnd, &rc );
    pResizer->_sizeMinTrack.cx = rc.right - rc.left;
    pResizer->_sizeMinTrack.cy = rc.bottom - rc.top;

    GetClientRect( hwnd, &rc );
    pResizer->_sizeMinClient.cx = rc.right - rc.left;
    pResizer->_sizeMinClient.cy = rc.bottom - rc.top;

    pResizer->_Prev.cx = rc.right - rc.left;
    pResizer->_Prev.cy = rc.bottom - rc.top;

    return wdjSubclass( resizeWndProc, hwnd, pResizer );
}

LRESULT CALLBACK resizeWndProc2(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( WM_DESTROY == msg ) {
        Resizer *pResizer = getResizer2( hwnd );
        freeResizer( pResizer );
    }
    return wdjCallOldProc( 
        resizeWndProc2, hwnd, msg, wParam, lParam );
}

BOOL makeResizable2( HWND hwnd, int idPivot ) {
    HWND hwndChild;
    RECT rcPivot;
    Resizer *pResizer;
    int iChild = 0;
    HWND hwndPivot = GetDlgItem( hwnd, idPivot );

    if ( !IsWindow( hwndPivot ) ) {
        return FALSE; //*** FUNCTION EXIT POINT
    }
    pResizer = calloc( 1, sizeof( Resizer ) );
    if ( 0 == pResizer ) {
        return FALSE; //*** FUNCTION EXIT POINT
    }

    for ( hwndChild = GetTopWindow( hwnd ); IsWindow( hwndChild );
        hwndChild = GetNextSibling( hwndChild ) )
    {
        pResizer->_nNumCtls++;
    }

    pResizer->_pCtlInfo = calloc( 
        pResizer->_nNumCtls, sizeof( CtlInfo ) );
    if ( 0 == pResizer->_pCtlInfo ) {
        free( pResizer );
        return FALSE; //*** FUNCTION EXIT POINT
    }
    GetWindowRect( hwndPivot, &rcPivot );

    hwndChild = GetTopWindow( hwnd );
    for ( iChild = 0; iChild < pResizer->_nNumCtls; ++iChild ) {
        CtlInfo *pCtlInfo = &pResizer->_pCtlInfo[ iChild ];
        RECT rcChild;
        GetWindowRect( hwndChild, &rcChild );
        pCtlInfo->_nCtlId = GetWindowID( hwndChild );
        pCtlInfo->_nHorzAlignment = none;
        if ( rcPivot.right <= rcChild.left ) {
            pCtlInfo->_nHorzAlignment |= left;
        }
        if ( rcPivot.right <= rcChild.right ) {
            pCtlInfo->_nHorzAlignment |= right;
        }
        pCtlInfo->_nVertAlignment = none;
        if ( rcPivot.bottom <= rcChild.top ) {
            pCtlInfo->_nVertAlignment |= top;
        }
        if ( rcPivot.bottom <= rcChild.bottom ) {
            pCtlInfo->_nVertAlignment |= bottom;
        }
        hwndChild = GetNextSibling( hwndChild );
    }

    if ( wdjSubclass( resizeWndProc2, hwnd, pResizer ) && 
         makeResizable( hwnd, pResizer ) )
    {
        return TRUE; //*** FUNCTION EXIT POINT
    }
    freeResizer( pResizer );
    return FALSE; //*** FUNCTION EXIT POINT
}

void setBehavior( HWND hwnd, int id, short horz, short vert) {
    Resizer *pResizer = getResizer( hwnd );
    if ( 0 != pResizer ) {
        int iChild;
        for ( iChild = 0; iChild < pResizer->_nNumCtls; ++iChild ) {
            CtlInfo *pCtlInfo = &pResizer->_pCtlInfo[ iChild ];
            if ( id == pCtlInfo->_nCtlId ) {
                pCtlInfo->_nHorzAlignment = horz;
                pCtlInfo->_nVertAlignment = vert;
                break; //*** LOOP EXIT POINT
            }
        }
    }
}
