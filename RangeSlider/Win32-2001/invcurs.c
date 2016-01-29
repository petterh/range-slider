#pragma warning( disable: 4115 ) // named typedef in parentheses

#include <windows.h>
#include "invcurs.h"

void APIENTRY invalidateCursor( void ) {

    POINT pt; // Screen coordinates!
    GetCursorPos( &pt );
    SetCursorPos( pt.x, pt.y );
}

// end of file
