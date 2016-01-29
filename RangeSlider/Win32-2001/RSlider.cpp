/*
 * RSlider.cpp:
 *
 * Implementation of Range Slider.
 * See http://pethesse.home.online.no/RangeSlider.html for details.
 *
 * This code was written by Petter Hesselberg. An earlier version was
 * published in the User Interface Programming column in Windows Developer
 * Magazine. There are no restrictions on the use of this code,
 * nor are there any guarantees for fitness of purpose or anything
 * else. Smile and carry your own risk :-)
 *
 * Changelog:
 *
 * Nov 2001     The implementation of VK_HOME and VK_END in onKey() was
 *              provided by Steve Conlan (mailto:ConlanS@SESLtd.com).
 */

#include "precomp.h"
#include <limits.h>
#include <assert.h>
#include "RSlider.h"
#include "invcurs.h"
#include "getClientRect.h"
#include "fillRect.h"


enum Metrics {
    DRAG_HANDLE_WIDTH = 2,
    FIRST_TIMEOUT     = 600,
    SECOND_TIMEOUT    = 200,
};


// TODO: escape key, or at least discuss! also WM_CTLCOLOR
// TODO: Sound effects on impossibilities?
// TODO: Paint depressed button and/or hot-tracking button
// TODO: Version number/cb member in RS_INFO


#undef HANDLE_WM_MOUSEWHEEL // buggy implementation in windowsx.h!
#ifndef HANDLE_WM_MOUSEWHEEL
#define HANDLE_WM_MOUSEWHEEL( hwnd, wParam, lParam, fn ) \
    ((fn)((hwnd), LOWORD(wParam), (int)(short) HIWORD(wParam),  \
    GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) ), 0L)
#endif


#define DEFINE_GET_SET_WINDOW_LONG( name, offs )       \
    static LONG get ## name( HWND hwnd ) {             \
        return GetWindowLong( hwnd, offs ); }          \
    static void set ## name( HWND hwnd, LONG value ) { \
        SetWindowLong( hwnd, offs, value ); }

DEFINE_GET_SET_WINDOW_LONG( Lower      ,  0 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( Upper      ,  1 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( Start      ,  2 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( End        ,  3 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( MinRange   ,  4 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( Granularity,  5 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( HTCode     ,  6 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( HTOffs     ,  7 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( SaveStart  ,  8 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( SaveEnd    ,  9 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( SaveStart2 , 10 * sizeof( LONG ) );
DEFINE_GET_SET_WINDOW_LONG( SaveEnd2   , 11 * sizeof( LONG ) );
#define WINDOW_BYTES                   ( 12 * sizeof( LONG ) )


enum htCode { htNone,
    htLeft, htLeftSlider, htSlider, htRightSlider, htRight,
};


inline void invalidateRect( HWND hwnd ) {

    InvalidateRect( hwnd, 0, TRUE );
}


inline bool isVertical( HWND hwnd ) {

    return 0 != (0x1 & GetWindowStyle( hwnd ));
}


static POINT getCursorPos( HWND hwnd ) {

    POINT pt = { 0 };
    GetCursorPos( &pt );
    ScreenToClient( hwnd, &pt );
    return pt;
}


inline LONG getHandle( HWND hwnd ) {

    const RECT rc = getClientRect( hwnd );
    return isVertical( hwnd ) ? rc.right - rc.left : rc.bottom - rc.top;
}


inline LONG getWidth( HWND hwnd ) {

    const RECT rc = getClientRect( hwnd );
    const int width = isVertical( hwnd ) ? rc.bottom - rc.top : rc.right - rc.left;
    return width - 2 * getHandle( hwnd );
}


inline LONG getRange( HWND hwnd ) {

    return getUpper( hwnd ) - getLower( hwnd );
}


inline LONG sliderFromPix( HWND hwnd, LONG x ) {

    return MulDiv( x, getRange( hwnd ), getWidth( hwnd ) );
}


inline LONG pixFromSlider( HWND hwnd, LONG x ) {

    return MulDiv( x, getWidth( hwnd ), getRange( hwnd ) );
}


static RECT getSliderRect( HWND hwnd ) {

    RECT rc = getClientRect( hwnd );
    const int pixStart = getHandle( hwnd ) + pixFromSlider( hwnd, getStart( hwnd ) - getLower( hwnd ) );
    const int pixEnd   = getHandle( hwnd ) + pixFromSlider( hwnd, getEnd  ( hwnd ) - getLower( hwnd ) );

    if ( isVertical( hwnd ) ) {
        rc.top    = pixStart;
        rc.bottom = pixEnd  ;
    } else {
        rc.left   = pixStart;
        rc.right  = pixEnd  ;
    }

    return rc;
}


static int hitTest( HWND hwnd, int x ) {

    const RECT rc = getSliderRect( hwnd );
    const int left  = isVertical( hwnd ) ? rc.top    : rc.left ;
    const int right = isVertical( hwnd ) ? rc.bottom : rc.right;

    if ( x < left - getHandle( hwnd ) ) {
        return htLeft;
    }
    if ( x <= left ) {
        return htLeftSlider;
    }
    if ( x < right ) {
        return htSlider;
    }
    if ( x <= right + getHandle( hwnd ) ) {
        return htRightSlider;
    }
    return htRight;
}


inline int hitTest( HWND hwnd ) {

    const POINT pt = getCursorPos( hwnd );
    return hitTest( hwnd, isVertical( hwnd ) ? pt.y : pt.x );
}


static void notifyParent( HWND hwnd, UINT code = 0 ) {         // NM_ code TODO

    NMRANGESLIDER nm = { { 0 } };
    
    nm.hdr.hwndFrom = hwnd;
    nm.hdr.idFrom = GetWindowID( hwnd );
    nm.hdr.code = code;

    nm.rsInfo._lower       = getLower      ( hwnd );
    nm.rsInfo._upper       = getUpper      ( hwnd );
    nm.rsInfo._start       = getStart      ( hwnd );
    nm.rsInfo._end         = getEnd        ( hwnd );
    nm.rsInfo._minRange    = getMinRange   ( hwnd );
    nm.rsInfo._granularity = getGranularity( hwnd );
    
    FORWARD_WM_NOTIFY(
        GetParent( hwnd ), nm.hdr.idFrom, &nm, SNDMSG );
}

// Discuss: onCreate, lpCreateStruct->lpCreateParams and createFunc, 
// more get/set messages, notifications

static int getBkColor( HWND hwnd ) {

    return IsWindowEnabled( hwnd ) ? COLOR_WINDOW : COLOR_BTNFACE;
}


static int getSliderColor( HWND hwnd ) {

    if ( IsWindowEnabled( hwnd ) && hwnd == GetFocus() ) {
        return COLOR_HIGHLIGHT;
    }
    return COLOR_BTNSHADOW;
}


static BOOL onCreate( HWND, const CREATESTRUCT *pCreateStruct ) {

    assert( 0 != (pCreateStruct->style & WS_CHILD) );
    return TRUE;
}


// TODO mouse tracking -- snap back when too far away?
// Note how this never overpaints, compared with code in magazine
static void onPaint( HWND hwnd ) {

    const int DFCS_HOT = 4096;

    const RECT rcSlider = getSliderRect( hwnd );
    const RECT rcClient = getClientRect( hwnd );
    RECT rcLeft = rcClient;

    if ( isVertical( hwnd ) ) {
        rcLeft.bottom = rcSlider.top;
    } else {
        rcLeft.right = rcSlider.left;
    }

    RECT rcRight = rcClient;
    if ( isVertical( hwnd ) ) {
        rcRight.top = rcSlider.bottom;
    } else {
        rcRight.left = rcSlider.right;
    }

    const int leftColor  = getHTCode( hwnd ) == htLeft  ? COLOR_BTNSHADOW : getBkColor( hwnd );
    const int rightColor = getHTCode( hwnd ) == htRight ? COLOR_BTNSHADOW : getBkColor( hwnd );

    PAINTSTRUCT ps = { 0 };
    BeginPaint( hwnd, &ps );
    fillRect( ps.hdc, hwnd, &rcLeft  , leftColor              );
    fillRect( ps.hdc, hwnd, &rcSlider, getSliderColor( hwnd ) );
    fillRect( ps.hdc, hwnd, &rcRight , rightColor             );

    if ( isVertical( hwnd ) ) {
        const int width = rcClient.right - rcClient.left;
        rcLeft .top    = rcLeft .bottom - width;
        rcRight.bottom = rcRight.top    + width;
    } else {
        const int height = rcClient.bottom - rcClient.top;
        rcLeft .left  = rcLeft .right - height;
        rcRight.right = rcRight.left  + height;
    }
    const UINT extraLeft  = getHTCode( hwnd ) == htLeftSlider  ? DFCS_HOT : 0;
    const UINT extraRight = getHTCode( hwnd ) == htRightSlider ? DFCS_HOT : 0;

    DrawFrameControl( ps.hdc, &rcLeft , DFC_SCROLL, extraLeft  | (isVertical( hwnd ) ? DFCS_SCROLLDOWN : DFCS_SCROLLRIGHT) );
    DrawFrameControl( ps.hdc, &rcRight, DFC_SCROLL, extraRight | (isVertical( hwnd ) ? DFCS_SCROLLUP   : DFCS_SCROLLLEFT ) );

    EndPaint( hwnd, &ps );
}


inline bool mouseInWindow( HWND hwnd ) {

    POINT pt = { 0 };
    GetCursorPos( &pt );
    return WindowFromPoint( pt ) == hwnd;
}


static long adjust( HWND hwnd, long value ) { // TODO: neg nums?

    const long granularity = getGranularity( hwnd );
    if ( 1 < granularity ) {
        value += (granularity - 1);
        value /= granularity;
        value *= granularity;
    }
    return value;
}


static void scroll( HWND hwnd, int direction, int timeOut ) {

    LONG page_size = getEnd( hwnd ) - getStart( hwnd );
    if ( page_size <= getGranularity( hwnd ) ) {
        page_size = getGranularity( hwnd );
    }
    if ( page_size <= 0 ) {
        page_size = 1;
    }

    LONG delta = 0;
    if ( -1 == direction ) {
        delta = -page_size;
        if ( getStart( hwnd ) + delta < getLower( hwnd ) ) {
            delta = getLower( hwnd ) - getStart( hwnd );
        }
    } else {
        delta = page_size;
        if ( getUpper( hwnd ) < getEnd( hwnd ) + delta ) {
            delta = getUpper( hwnd ) - getEnd( hwnd );
        }
    }

    int start = adjust( hwnd, getStart( hwnd ) + delta );
    int end   = adjust( hwnd, getEnd  ( hwnd ) + delta );
    if ( start != getStart( hwnd ) || end != getEnd( hwnd ) ) {
        setStart( hwnd, start );
        setEnd  ( hwnd, end   );
        invalidateRect( hwnd );
        invalidateCursor();
        notifyParent( hwnd );
    }

    // TODO: discuss SPI_GETKEYBOARDDELAY
    if ( 0 < timeOut ) {
        SetTimer( hwnd, 1, timeOut, 0 );
    }
}


static void scroll( HWND hwnd, int timeOut ) {

    if ( getHTCode( hwnd ) == hitTest( hwnd ) ) {
        switch ( getHTCode( hwnd ) ) {
        case htLeft:
            scroll( hwnd, -1, timeOut );
            break;
        case htRight:
            scroll( hwnd, 1, timeOut );
            break;
        }
    }
}

static void onTimer( HWND hwnd, UINT id ) {

    scroll( hwnd, SECOND_TIMEOUT );
}

#if 0
OCX Notes
�   Enabling and disabling the control does not send WM_ENABLE message,
    but rather invokes EnableWindow() method on it.
�   Incompatibilities with design-time props (Win9x, NT+2000)
�   Forces CS_PARENTDC and CS_DBLCLKS.
�   Much overkill - registry, servers, blabla.
�   When the underlying OCX changes, it's a real problem to regenerate the wrapper class.

Discuss
Use of onEraseBkgnd()
DragDetect()
Resizebehavior: CS_HREDRAW | CS_VREDRAW is the simple solution.
#endif

// TODO -- integrate handling of mouse and keyboard
// TODO -- mouse wheel? vertical? boundary values? doubleclick (min/max)
// TODO -- if not focus, just SetFocus() on click? discuss!
static void onLButtonDown(
    HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
    const RECT rc = getSliderRect( hwnd );

    SetFocus( hwnd );
    SetCapture( hwnd );
    setHTCode( hwnd, hitTest( hwnd, isVertical( hwnd ) ? y : x ) );
    setSaveStart( hwnd, getStart( hwnd ) );
    setSaveEnd  ( hwnd, getEnd  ( hwnd ) );

    switch ( getHTCode( hwnd ) ) {
    case htSlider:
        if ( fDoubleClick ) {
            if ( getStart( hwnd ) == getLower( hwnd ) && getEnd( hwnd ) == getUpper( hwnd ) ) {
                setStart( hwnd, getSaveStart2( hwnd ) );
                setEnd  ( hwnd, getSaveEnd2  ( hwnd ) );
            } else {
                setSaveStart2( hwnd, getStart( hwnd ) );
                setSaveEnd2  ( hwnd, getEnd  ( hwnd ) );
                setStart( hwnd, getLower( hwnd ) );
                setEnd  ( hwnd, getUpper( hwnd ) );
            }

            invalidateRect( hwnd );
            invalidateCursor(); // TODO discuss (relate to IE problem)
            notifyParent( hwnd ); // TODO: reversal
            ReleaseCapture();
            break;
        }
        //*** FALL THROUGH
    case htLeftSlider:
        setHTOffs( hwnd, isVertical( hwnd ) ? y - rc.top : x - rc.left );
        break;
    case htRightSlider:
        setHTOffs( hwnd, isVertical( hwnd ) ? y - rc.bottom : x - rc.right );
        break;
    case htLeft:
    case htRight:
        scroll( hwnd, FIRST_TIMEOUT );
        break;
    }
}


// TODO -- this thing is ugly. What's the best refactoring?
// Come to think of it, onLButtonDown() ain't too pleasing either.
static void onMouseMove( HWND hwnd, int x, int y, UINT flags ) {

    if ( GetCapture() != hwnd ) {
        return; //*** FUNCTION EXIT POINT
    }

    if ( isVertical( hwnd ) ) {
        y -= getHTOffs( hwnd );
    } else {
        x -= getHTOffs( hwnd );
    }

    long start = getStart( hwnd );
    long end   = getEnd  ( hwnd );

    switch ( getHTCode( hwnd ) ) {
    case htLeftSlider: // TODO sliderFromPix -- send in whole pt?
        start = getLower( hwnd ) + sliderFromPix( hwnd, (isVertical( hwnd ) ? y : x) - getHandle( hwnd ) );
        if ( start < getLower( hwnd ) ) {
            start = getLower( hwnd );
        }
        if ( end < start + getMinRange( hwnd ) ) {
            if ( 0 == getMinRange( hwnd ) ) {
                end = start;
                if ( getUpper( hwnd ) < start ) {
                    end = start = getUpper( hwnd );
                }
            } else {
                start = end - getMinRange( hwnd );
            }
        }
        break;
    case htRightSlider:
        end = getLower( hwnd ) + sliderFromPix( hwnd, (isVertical( hwnd ) ? y : x) - getHandle( hwnd ) );
        if ( getUpper( hwnd ) < end ) {
            end = getUpper( hwnd );
        }
        if ( end < start + getMinRange( hwnd ) ) {
            if ( 0 == getMinRange( hwnd ) ) {
                start = end;
                if ( end < getLower( hwnd ) ) {
                    start = end = getLower( hwnd );
                }
            } else {
                end = start + getMinRange( hwnd );
            }
        }
        break;
    case htSlider:
        start = getLower( hwnd ) + sliderFromPix( hwnd, (isVertical( hwnd ) ? y : x) - getHandle( hwnd ) );
        if ( start < getLower( hwnd ) ) {
            start = getLower( hwnd );
        }
        end += start - getStart( hwnd );
        if ( getUpper( hwnd ) < end ) {
            start -= end - getUpper( hwnd );
            end = getUpper( hwnd );
        }
        break;
    }
    start = adjust( hwnd, start );
    end   = adjust( hwnd, end   );
    if ( start != getStart( hwnd ) || end != getEnd( hwnd ) ) {
        setStart( hwnd, start );
        setEnd  ( hwnd, end );
        invalidateRect( hwnd ); // TODO: invalidate ranges
        notifyParent( hwnd );
    }
}


static void onLButtonUp( HWND hwnd, int x, int y, UINT flags ) {

    ReleaseCapture();
    if ( htNone != getHTCode( hwnd ) ) {
        setHTCode( hwnd, htNone );
        invalidateRect( hwnd );
    }
    KillTimer( hwnd, 1 );
            // TODO notify parent
}


static void onKey(
    HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
{
    long start = getStart( hwnd );
    long end   = getEnd( hwnd );

    const BOOL controlKey = GetAsyncKeyState( VK_CONTROL ) < 0;

    if ( VK_ESCAPE == vk ) {
        if ( htNone != getHTCode( hwnd ) ) {
            setHTCode( hwnd, htNone );
            const LONG oldStart = getSaveStart( hwnd );
            const LONG oldEnd   = getSaveEnd  ( hwnd );
            setStart( hwnd, oldStart );
            setEnd  ( hwnd, oldEnd   );
            invalidateRect( hwnd );
            //onLButtonUp( hwnd, 0, 0, 0 ); // TODO retain capture anyway?
            // TODO notify parent
            return; //** FUNCTION EXIT POINT
        }
    }

    const UINT left_key  = isVertical( hwnd ) ? VK_UP   : VK_LEFT ;
    const UINT right_key = isVertical( hwnd ) ? VK_DOWN : VK_RIGHT;

    long granularity = getGranularity( hwnd );
    if ( granularity <= 0 ) {
        granularity = 1;
    }

    if ( left_key == vk ) {
        if ( controlKey ) {
            if ( getMinRange( hwnd ) < end - start ) {
                end -= granularity;
            }
        } else if ( getLower( hwnd ) < start ) {
            start -= granularity;
            end   -= granularity;
        }
    } else if ( right_key == vk ) {
        if ( end < getUpper( hwnd ) ) {
            end += granularity;
            if ( !controlKey ) {
                start += granularity;
            }
        }
    } else if ( VK_PRIOR == vk ) {
        scroll( hwnd, -1, 0 );
        return; //*** FUNCTION EXIT POINT
    } else if ( VK_NEXT == vk ) {
        scroll( hwnd, 1, 0 );
        return; //*** FUNCTION EXIT POINT
    } else if ( VK_HOME == vk ) {
        const long range = abs( getLower( hwnd ) - start );
        start -= range;
        end   -= range;
    } else if ( VK_END == vk ) {
        const long range = abs( getUpper( hwnd ) - end );
        start += range;
        end   += range;
    }

    start = adjust( hwnd, start );
    end   = adjust( hwnd, end   );
    if ( start != getStart( hwnd ) || end != getEnd( hwnd ) ) {
        setStart( hwnd, start );
        setEnd  ( hwnd, end );
        invalidateRect( hwnd );
        invalidateCursor();
        notifyParent( hwnd );
    }
}


static void onMouseWheel(
    HWND hwnd, UINT fwKeys, int zDelta, int x, int y )
{
    if ( 0 != fwKeys ) {
        ;
    } else {
        UINT lines_to_scroll = 0;
        SystemParametersInfo(
            SPI_GETWHEELSCROLLLINES, 0, &lines_to_scroll, 0 );
        if ( WHEEL_PAGESCROLL == lines_to_scroll ) {
            if ( zDelta < 0 ) {
                scroll( hwnd, 1, 0 );
            } else if ( 0 < zDelta ) {
                scroll( hwnd, -1, 0 );
            }
        } else if ( 0 < lines_to_scroll ) {
            // TODO -- adjust for page size/granularity
            if ( zDelta < 0 ) {
                const UINT right_key = isVertical( hwnd ) ? VK_DOWN : VK_RIGHT;
                for ( UINT line = 0; line < lines_to_scroll; ++line ) {
                    onKey( hwnd, right_key, true, 1, 0 );
                }
            } else if ( 0 < zDelta ) {
                const UINT left_key = isVertical( hwnd ) ? VK_UP : VK_LEFT;
                for ( UINT line = 0; line < lines_to_scroll; ++line ) {
                    onKey( hwnd, left_key, true, 1, 0 );
                }
            }
        }
    }
}


static void onSetKillFocus( HWND hwnd, HWND hwnUnused ) {

    invalidateRect( hwnd );
}


static UINT onGetDlgCode( HWND hwnd, LPMSG lpmsg ) {

    UINT dlgCode = DLGC_WANTARROWS;
    if ( htNone != getHTCode( hwnd ) ) {
        dlgCode |= DLGC_WANTALLKEYS;
    }
    return dlgCode;
}

static BOOL isSizeCursor( int htCode ) {

    return false; // htLeftSlider == htCode || htRightSlider == htCode;
}


static BOOL onSetCursor(
    HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg )
{
    LPCTSTR cursor = IDC_ARROW;
    const int htCode = hitTest( hwnd );
    if ( HTCLIENT == codeHitTest && isSizeCursor( htCode ) ) {
        cursor = isVertical( hwnd ) ? IDC_SIZENS : IDC_SIZEWE;
    }
    SetCursor( LoadCursor( 0, cursor ) );
    return TRUE;

}


static void onEnable( HWND hwnd, BOOL fEnable ) {

    invalidateRect( hwnd );
}


static void onSize( HWND hwnd, UINT state, int cx, int cy ) {

    invalidateRect( hwnd );
}


inline bool contectMenuActivatedByKeyboard( UINT xPos, UINT yPos ) {

    return (WORD) -1 == (WORD) xPos
        || (WORD) -1 == (WORD) yPos;
}


static void getContextMenuPosition( HWND hwnd, UINT *pxPos, UINT *pyPos ) {

    RECT rcWindow = { 0 };
    GetWindowRect( hwnd, &rcWindow );
    const int offset = isVertical( hwnd ) 
        ? rcWindow.right - rcWindow.left 
        : rcWindow.bottom - rcWindow.top;
    *pxPos = rcWindow.left + offset / 2;
    *pyPos = rcWindow.top  + offset / 2;
}


#include "resource.h"
static void onContextMenu( HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos ) {

    if ( GetCapture() == hwnd ) { // Not while dragging...? TODO -- abort dragging?
        return; //*** FUNCTION EXIT POINT
    }
    SetFocus( hwnd ); // Is this OK? NO! TODO!

    if ( contectMenuActivatedByKeyboard( xPos, yPos ) ) {
        getContextMenuPosition( hwnd, &xPos, &yPos );
    }

#if 0
#if 0   // TODO -- how to handle menu?
        // Store menu as such, or store strings, populate menu dynamically?
        // How to handle translations? Static/dynamic linking?
    HMENU hmenu = CreatePopupMenu();
    if ( 0 != hmenu ) {
        ; // Add items
    }
#else
    HMENU hmenu = LoadMenu( GetWindowInstance( hwnd ), MAKEINTRESOURCE( IDM_RANGESLIDER ) );
    TrackPopupMenu( GetSubMenu( hmenu, 0 ), 
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
        xPos, yPos, 0,
        hwnd, 0 );
#endif
    DestroyMenu( hmenu );
#endif
}


static void onCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify ) {

    switch ( id ) {
    case ID_MAXIMIZE:
        FORWARD_WM_LBUTTONDOWN( hwnd, TRUE, 0, 0, 0, SNDMSG );
        break;
    }
}


#define HANDLE_RSN_SETINFO( hwnd, wParam, lParam, fn ) \
    ((fn)((hwnd), (const RS_INFO *) (wParam)), 1L)

static void onRsnSetInfo( HWND hwnd, const RS_INFO *pInfo ) {

    setLower      ( hwnd, pInfo->_lower       );
    setUpper      ( hwnd, pInfo->_upper       );
    setSaveStart2 ( hwnd, pInfo->_lower       );
    setSaveEnd2   ( hwnd, pInfo->_upper       );
    setStart      ( hwnd, pInfo->_start       );
    setEnd        ( hwnd, pInfo->_end         );
    setMinRange   ( hwnd, pInfo->_minRange    );
    setGranularity( hwnd, pInfo->_granularity );

    invalidateRect( hwnd );
    invalidateCursor();
}


static LRESULT WINAPI rangeSliderWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg ) {

    HANDLE_MSG( hwnd, WM_CREATE       , onCreate       );
    HANDLE_MSG( hwnd, WM_PAINT        , onPaint        );
    HANDLE_MSG( hwnd, WM_LBUTTONDOWN  , onLButtonDown  );
    HANDLE_MSG( hwnd, WM_LBUTTONDBLCLK, onLButtonDown  );
    HANDLE_MSG( hwnd, WM_TIMER        , onTimer        );
    HANDLE_MSG( hwnd, WM_MOUSEMOVE    , onMouseMove    );
    HANDLE_MSG( hwnd, WM_LBUTTONUP    , onLButtonUp    );
    HANDLE_MSG( hwnd, WM_KEYDOWN      , onKey          );
    HANDLE_MSG( hwnd, WM_MOUSEWHEEL   , onMouseWheel   );
    HANDLE_MSG( hwnd, WM_SETFOCUS     , onSetKillFocus );
    HANDLE_MSG( hwnd, WM_KILLFOCUS    , onSetKillFocus );
    HANDLE_MSG( hwnd, WM_GETDLGCODE   , onGetDlgCode   );
    HANDLE_MSG( hwnd, WM_SETCURSOR    , onSetCursor    );
    HANDLE_MSG( hwnd, WM_ENABLE       , onEnable       );
    HANDLE_MSG( hwnd, WM_SIZE         , onSize         );
    HANDLE_MSG( hwnd, WM_CONTEXTMENU  , onContextMenu  );
    HANDLE_MSG( hwnd, WM_COMMAND      , onCommand      );
    HANDLE_MSG( hwnd, RSN_SETINFO     , onRsnSetInfo   );

    } // msg-switch.

    return DefWindowProc( hwnd, msg, wParam, lParam );
}


extern "C" BOOL initRangeSlider( HINSTANCE hinst ) {

    WNDCLASS wndClass = { 0 };
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = rangeSliderWndProc;
    wndClass.cbWndExtra = WINDOW_BYTES;
    wndClass.hInstance = hinst;
    wndClass.lpszClassName = _T( "wdmRangeSlider" );
    return RegisterClass( &wndClass );
}


extern "C" BOOL setSliderRange(
    HWND hwnd, LONG lower, LONG upper, LONG start, LONG end, LONG minRange, LONG granularity )
{
    assert( IsWindow( hwnd ) ); // TODO: assert correct class?
    RS_INFO rsInfo = {
        lower, upper, start, end, minRange, granularity,
    };
    return IsWindow( hwnd ) && SendMessage( hwnd, RSN_SETINFO, (WPARAM) &rsInfo, 0 );
}


extern "C" BOOL setDlgItemSliderRange(
    HWND hwnd, int id, LONG lower, LONG upper, LONG start, LONG end, LONG minRange, LONG granularity )
{
    HWND hwndRangeSlider = GetDlgItem( hwnd, id );
    return setSliderRange(
        hwndRangeSlider, lower, upper, start, end, minRange, granularity );
}

// end of file
