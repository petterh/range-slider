#ifdef __cplusplus
extern "C" {
#endif

#define RSN_SETINFO   WM_USER
#define RSN_SETBOTTOM WM_USER + 1
#define RSN_SETTOP    WM_USER + 2
#define RSN_SETLOWER  WM_USER + 3
#define RSN_SETUPPER  WM_USER + 4

typedef struct _RS_INFO {
    LONG _lower;
    LONG _upper;
    LONG _start;
    LONG _end;
    LONG _minRange;
    LONG _granularity;
} RS_INFO;

typedef struct _NMRANGESLIDER {
    NMHDR   hdr;
    RS_INFO rsInfo;
} NMRANGESLIDER;

BOOL initRangeSlider( HINSTANCE hinst );

HWND WINAPI createRangeSlider(
    DWORD style, int x, int y, int cx, int cy,
    HWND hwndParent, int id, HINSTANCE hinst,
    int lower, int upper, int start, int end, int minDelta, int granularity );

BOOL setSliderRange(
    HWND hwnd, LONG lower, LONG upper, LONG start, LONG end, LONG minRange, LONG granularity );
BOOL setDlgItemSliderRange(
    HWND hwnd, int id, LONG lower, LONG upper, LONG start, LONG end, LONG minRange, LONG granularity );

#ifdef __cplusplus
}
#endif

// end of file
