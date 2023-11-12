//#define DO_OEM "Weasel"
//#define OEM_COMPANY "RIX "

//#define _THEDISC

#if  _MSC_VER >= 1400
#define MYBETA
#define MYVER 694   // so that keys still work
#define MYFILEVERSION 6,9,4,0
#define MYPRODVERSION 6,9,4,0
#define MYSTRFILEVERSION "6.94\0"
#define MYSTRPRODVERSION "6.94\0"
#else
#define MYVER 640
#define MYFILEVERSION 6,4,0,0
#define MYPRODVERSION 6,4,0,0
#define MYSTRFILEVERSION "6.4.0\0"
#define MYSTRPRODVERSION "6.4.0\0"
#endif
#ifdef _THEDISC
#define DO_OEM "TheDISC!"
#define OEM_COMPANY ""
#define MYCOMPANY "Animaction, Inc.\0"
#define MYFILEDESC "TheDISC! MFC Application\0"
#define MYINTERNAL "TheDISC!\0"
#define MYORIGNAME "TheDISC!.EXE\0"
#define MYPRODUCT  "TheDISC!\0"
#define MYEXTNAME  "The DISC! 5"
#else
#define MYCOMPANY "DigiCel, Inc.\0"
#define MYFILEDESC "FlipBook MFC Application\0"
#define MYINTERNAL "FLIPBOOK\0"
#define MYORIGNAME "FLIPBOOK.EXE\0"
#define MYPRODUCT  "FlipBook\0"
#define MYEXTNAME  "FlipBook 6"
#endif
