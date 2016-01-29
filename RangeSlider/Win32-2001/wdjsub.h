/*
 * wdjsub.h: 
 * Declarations for subclassing functions defined in wdjsub.c.
 *
 * See http://www.wdj.com/archive/1103/feature.html for details,
 * plus further comments in wdjsub.c.
 */

#ifdef __cplusplus
extern "C" {
#endif

BOOL    wdjSubclass    ( WNDPROC wndProc, HWND hwnd, void *pData );
BOOL    wdjUnhook      ( WNDPROC id, HWND hwnd );
LRESULT wdjCallOldProc ( WNDPROC id, HWND hwnd, 
                         UINT msg, WPARAM wParam, LPARAM lParam );
void *  wdjGetData     ( WNDPROC id, HWND hwnd );
BOOL    wdjSetData     ( WNDPROC id, HWND hwnd, void *pData );
BOOL    wdjIsSubclassed( WNDPROC id, HWND hwnd );

#ifdef _DEBUG
void __cdecl tracef( LPCTSTR pszFmt, ... );
#endif

#ifdef __cplusplus
}
#endif

// end of file
