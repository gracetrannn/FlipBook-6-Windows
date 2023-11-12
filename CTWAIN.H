///////////////////////////////////////////////////////////////////////
//
// ctwain.h - application interface to TWAIN Protocol
//
// Version 1.51  1999.01.31
// Copyright (C) 1998-1999 Dosadi.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
#ifndef CTWAIN_H
#define CTWAIN_H

#ifdef FLIPBOOK_MAC
#include <TWAIN/TWAIN.h>
#else
#include "twain.h"
#endif

//           ------- Architectural & Usage Notes -------
//
// We advise adding CTwain as a 2nd base class to the main frame
// window.  This will give you immediate access to SelectSource and
// ModalAcquire.
// The classic user interface for TWAIN is to add Select Source...
// and Acquire... commands to the File menu.
//
// Override CTwain::DibReceived to receive the acquired images (DIBs).
// When you call ModalAcquire, any transfers will take place before
// ModalAcquire returns.
//
//                ---------- Refinements -----------
//
// At some point, you should call RegisterApp with real info.
//
// If !TwainAvailable(), forget the rest, TWAIN not installed on host.
//
// A reasonable way to enable the Acquire and Select Source commands
// is to enable them if (TwainAvailable() && State() < SOURCE_OPEN)
// This disables them if TWAIN is not installed, or if an Acquire is
// in progress.
//
// If it is appropriate for your application, handle asynchronous
// image transfers like this:
//
//   1. Override CWnd::PreTranslateMessage(pMsg) and do this:
//        return (TwainMessageHook(pMsg) ||
//                <base class>::PreTranslateMessage(pMsg);
//
//   2. On an Acquire command, instead of ModalAcquire, call BeginAcquire.
//      After BeginAcquire returns, you may receive images via DibReceived,
//      as long as the data source is left open.  Usually the user will
//      close the source, but you can call CloseDsRequest().
//
// If you want to acquire without showing the data source's user interface,
// call SetShowUI(FALSE) any time before starting acquisition - that is, before
// the source is enabled.  Please be aware that many data sources do not
// handle this correctly - quite a few ignore the request and show their
// user interface no matter what, and others hide their user interface but
// then won't scan!
//
// To control scan parameters such as resolution, depth, etc., you must do
// what TWAIN calls 'capability negotiation'.  This typically takes place in
// state 4 (SOURCE_OPEN), just before the source is enabled for scanning.
// While capability negotiation in general is beyond the scope of this note,
// I have provided a few basic methods - see 'Capability Negotiation' below.
// Override the NegotiateCapabilities() method - the sample app shows how to do it.
//

typedef enum {
	NO_TWAIN_STATE,			// 0	internal use only
	PRE_SESSION,			   // 1	ground state, nothing loaded
	SOURCE_MANAGER_LOADED,	// 2	DSM loaded but not open
	SOURCE_MANAGER_OPEN,	   // 3	DSM open
	SOURCE_OPEN,			   // 4	some Source open - Negotiation state!
	SOURCE_ENABLED,			// 5	acquisition started
	TRANSFER_READY,			// 6	data ready to transfer
	TRANSFERRING			   // 7	transfer started
} TW_STATE;

typedef enum {
	TWERR_OPEN_DSM,			// unable to load or open Source Manager
	TWERR_OPEN_SOURCE,		// unable to open Datasource
	TWERR_ENABLE_SOURCE,	   // unable to enable Datasource
   TWERR_NOT_4,            // capability set outside state 4 (SOURCE_OPEN)
   TWERR_CAP_SET,          // capability set failed
} TW_ERR;

class CTwain
{
public:
	CTwain(void);
	~CTwain();

// General notes:
// 1. Once the source manager has been loaded, it is not normally unloaded
// until this object is destroyed, or until you call UnloadSourceManager.
// This makes normal TWAIN operations quite a bit faster.  Calling TwainAvailable
// the first time loads the source manager.

// attributes i.e. query functions

   enum { VERSION = 151 };
   // Major Rev * 100 + Minor Rev, access as CTwain::VERSION

	int TwainAvailable(void);
	// TRUE if the TWAIN Datasource Manager is available and can
	// be loaded.  Does not check that there are any datasources!
	// But normally, the presence of the DSM means that at least one
	// datasource has been installed.
	// IsAvailable is fast after the first call - you can use it
	// to enable or disable menu items for example.

	TW_STATE State(void);
	// Return the current (presumed) state of the Twain connection.

	unsigned ResultCode(void);
	// last result code (see twain.h)

	unsigned ConditionCode(void);
	// retrieve condition code from last triplet - see twain.h.

// Top-level operations
	void SetDefWindow(CWnd* pWnd);
	// Sets the CWnd* to be used as the default for all other calls that
	// take a window argument - SelectSource, OpenSourceManager, etc.
	// If no default window is set, AfxGetMainWnd() is used.


	int SelectSource(CWnd* pWnd = NULL);
	// Post the standard Select Source dialog

	void ModalAcquire(CWnd* pWnd = NULL);
	// Acquire images from current or default source.
	// Start acquisition from current or default datasource.
	// If a source is open, uses that source - otherwise the
	// default source is opened.
	// By default, displays the u/i of the source and acquires images
	// until a CLOSEDSREQ is received from the source.
	// Acquire returns the Twain connection to the starting state
	// or to SOURCE_ENABLED, whichever is lower.

	int BeginAcquire(CWnd* pWnd = NULL);
	// Open and enable the default source.
	// TRUE if successful, FALSE if something goes wrong.
	// If successful, State() == SOURCE_ENABLED.
	// You must now pass all messages to MessageHook until the state
	// drops below SOURCE_OPEN: In MFC, override PreTranslateMessage.
	// If BeginAcquire fails, it returns TWAIN to the state it
	// found it in.

	void RegisterApp(						// Record application information
		int		nMajorNum, int nMinorNum,	// major and incremental revision of application. E.g.
											// for version 2.1, nMajorNum == 2 and nMinorNum == 1
		int		nLanguage,					// language of this version (use TWLG_xxx from TWAIN.H)
		int		nCountry,					// country of this version (use TWCY_xxx from TWAIN.H)
		LPSTR	lpszVersion,				// version info string e.g. "1.0b3 Beta release"
		LPSTR	lpszMfg,					// name of manufacturer/developer e.g. "Crazbat Software"
		LPSTR	lpszFamily,					// product family e.g. "BitStomper"
		LPSTR	lpszProduct);				// specific product e.g. "BitStomper Deluxe Pro"
   // This is not necessary for acquisition, but is required for full
   // TWAIN compliance.

// Level 2 operations - more detailed control of Twain State.
	int OpenSourceManager(CWnd* pWnd = NULL);
	// (Load and) open the Datasource Manager
	// Loads the DSM if necessary, opens it if necessary.
	// If State >= 3, does nothing and returns TRUE.

	int OpenDefaultSource(void);
	// Open the default datasource (last source selected in Select Source dialog.)
	// Invalid (returns FALSE) if State > 3 (a source is already open)
	// Invalid (returns FALSE) if State < 3 (DSM is not open)
	// If successful, returns TRUE with State == 4.
	// Otherwise, returns FALSE with State unchanged.

	void SetShowUI(BOOL bShow = 1);
	// Set flag for whether source should be enabled with
	// user interface visible (bShow == TRUE) or not.
	// At construction, ShowUI is set TRUE.

	BOOL GetShowUI(void);
	// Return state of ShowUI flag.

   //--------- Capability Negotiation ----------------------------

   int SetCurrentResolution(double dRes);
   int SetCurrentPixelType(int nType);

   int SetBrightness(double dBri);
   // Note: Valid range is -1000.0 to +1000.0
   int SetContrast(double dCon);
   // Note: Valid range is -1000.0 to +1000.0

   int SetXferCount(int nXfers);

   int SetCurrentUnits(unsigned uUnits);
   unsigned GetCurrentUnits(void);
   // Set/Get current unit of measure (TWUN_INCHES, ... - see twain.h)
   // Several TWAIN functions take measurements - these are always
   // interpreted in the current unit of measure.  See e.g. SetImageLayout.

   int SetCapFix32(unsigned Cap, double dVal);
   int SetCapOneValue(unsigned Cap, TW_FIX32 ItemVal);
   int SetCapOneValue(unsigned Cap, unsigned ItemType, TW_UINT32 ItemVal);

   int SetImageLayout(double left, double top, double width, double height);
   // Set the region of the next image to be transferred.
   // Only valid in state 4 (SOURCE_OPEN)
   // All measurements are in the current units (see GetCurrentUnits)
   // This is a suggestion to the source, it does not have to comply.
   int GetImageLayout(double &left, double &top, double &width, double &height);
   // Get the current image layout.
   // Only valid in states >= 4 (SOURCE_OPEN & above)
   // All measurements are in the current units.

   BOOL GetCapCurrent(unsigned Cap, short& v);
   BOOL GetCapCurrent(unsigned Cap, unsigned short& v);
   BOOL GetCapCurrent(unsigned Cap, int& v);
   BOOL GetCapCurrent(unsigned Cap, unsigned& v);
   BOOL GetCapCurrent(unsigned Cap, double& v);
   BOOL GetCapCurrent(unsigned Cap, unsigned ItemType, void FAR *pVal);

   TW_FIX32 static ToFix32(double r);
   // Convert a float value to TWAIN's 32-bit fixed point format

   double static Fix32ToFloat(TW_FIX32 fix);
   // Convert a TWAIN fixed-point value to floating point.

   //--------- State Change Operations --------------------------

	int EnableSource(CWnd* pWnd = NULL);
	// Enable the open source, which allows image acquisition to begin.
	// Invalid if State != 4 (source open).
	// If successful, returns TRUE with State == 5.
	// Otherwise, returns FALSE with State unchanged.

	int DisableSource(void);
	// Disable the current source.
	// If State == 5, disables the current source and returns TRUE.
	// If State < 5, does nothing and returns TRUE.
	// If State > 5, does nothing and returns FALSE.

	int CloseSource(void);
	int CloseSourceManager(CWnd* pWnd = NULL);
	int UnloadSourceManager(void);

	int DropToState(int nS, CWnd* pWnd = NULL);

// low-level primitives
 	int LoadSourceManager(void);
	// loads the DSM (Datasource Manager) into process space

	void ModalEventLoop(void);
	// get and dispatch messages until source is disabled.

	int TwainMessageHook(LPMSG lpmsg);

	int EndXfer(void);
	// In State 7, ends the current transfer by sending MSG_ENDXFER.
	// If successful, goes to State 6 if there are more transfers
	// available (pendingXfers != 0), or to State 5 if not.
	// Returns TRUE if the resulting State < 7.

	int CancelXfers(void);
	// In State 6, cancels any pending transfers.
	// (In State 7, does an EndXfer first)
	// If successful, goes to State 5.
	// Returns TRUE if the resulting State < 6.

// bottom-level primitives
	int DS(unsigned long dg, unsigned dat, unsigned msg, void FAR *pd);
	// send a triplet to the current Datasource.
	// returns TRUE if the result code (rc) == RC_SUCCESS, FALSE otherwise.
	// Note that this is not meaningful with some triplets.
	// Does ASSERT(nState < 4);

	int SM(unsigned long dg, unsigned dat, unsigned msg, void FAR *pd);
	// Send a triplet to the Source Manager.
	// returns TRUE if the result code (rc) == RC_SUCCESS, FALSE otherwise.
	// Note that this is not meaningful with some triplets.
	// Does ASSERT(nState > 1);

//-------- call-backs
   virtual int NegotiateCapabilities(void);
   // called after source is successfully opened.
   // Use this call-back to negotiate any special settings
   // (capabilities) for the session.  Return TRUE if successful.
   // The default method just returns TRUE.

	virtual void XferReady(LPMSG lpmsg);
	// called when source has one or more xfers ready
	// This default method assumes XferMech = Native,
	// gets each native (DIB) image and calls back to DibReceived.

	virtual void CloseDsRequest(void);
	// called when the open source asks to be 'closed'
	// It is sufficient to disable the source on this request,
	// but this default handler calls CloseSource.

	virtual void DibReceived(HGLOBAL hDib);
	// called by the default XferReady handler
	// when it has successfully transferred a DIB.
	// This default handler just calls GlobalFree(hDib).

	virtual void StateChange(int nState);
	// called after each Twain State transition.
	// nState is the new State.  When this callback
	// occurs, the state transition has already happened.
	// Note - first call is the transition to State 1
	// which occurs at construction.

	virtual void TwainError(TW_ERR e);
	// Called when an unexpected TWAIN malfunction occurs.
	// See TW_ERR declaration at beginning of this file.

        BOOL Tracing(void) { return bTrace; }
        void Trace(BOOL bOn = TRUE) { bTrace = bOn; }

private:

	void SetState(TW_STATE nS);				// assume Twain in State nS

	CWnd* DefWnd(CWnd* pWnd = NULL);
	// Returns pWnd if it's non-null, otherwise it
	// finds a safe non-null default substitute.

	BOOL              bTrace;        // enable TRACE output
	TW_STATE          nState;        // current state
	TW_STATE          nStartState;   // starting state for some operation
	TW_IDENTITY       AppId;         // application identity structure
	TW_INT16          rc;            // last result code       
   TW_USERINTERFACE  twUI;
	BOOL              bShowUI;
   TW_PENDINGXFERS   pendingXfers;
	HANDLE            hDib;          // bitmap returned by native transfer
	CWnd*             pDefWnd;       // default window
};


// Some useful helpers
double ppm2dpi(long ppm);
// converts pels/meter (ppm) to dots/inch (dpi)

long dpi2ppm(double dpi);
// converts dpi to ppm

#endif
