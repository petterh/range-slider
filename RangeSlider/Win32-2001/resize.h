#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum Alignments {
    none  = 0, 
    left  = 1, top    = left, 
    right = 2, bottom = right, 
    both  = left | right,
};

#pragma pack( 1 )

typedef struct CtlInfo {
    int             _nCtlId;
    enum Alignments _nHorzAlignment;
    enum Alignments _nVertAlignment;
} CtlInfo;

typedef struct Resizer {
    int      _nNumCtls;
    CtlInfo  *_pCtlInfo;

    SIZE _Prev         ;
    SIZE _sizeMinTrack ;
    SIZE _sizeMinClient;
} Resizer;

#pragma pack()

BOOL makeResizable( HWND hwnd, Resizer *pResizer );

#ifdef __cplusplus
}
#endif
