inline RECT getClientRect( HWND hwnd ) {

    RECT rc = { 0 };
    GetClientRect( hwnd, &rc );
    return rc;
}

// end of file
