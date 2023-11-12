#if defined(IMPLEMENT_WINDOWS_APIS) || defined(__APPLE__)

#include "CString.h"
#include "Resource.h"
#include <cstdarg>

CString::CString(): std::string()
{
}

CString::CString(const char* str): std::string(str)
{
}

CString::operator char* () const
{
	return (char*)c_str();
}

CString::operator const char* () const
{
    return c_str();
}

void CString::ChangeSuffix(std::string newSuffix)
{
	size_t pos = find_last_of('.');
	if (pos != npos) {
		size_t oldSuffixLength = length() - pos;
		replace(pos, oldSuffixLength, newSuffix);
	}
}

void CString::Format(const char* inFormat, ...)
{
	char* s = new char[1024]; // hope this is enough
	va_list args;
	va_start(args, inFormat);
	vsprintf(s, inFormat, args);
	va_end(args);
	assign(s);
	delete[] s;
}

bool CString::LoadString(unsigned int id)
{
    if (id == IDS_VERSION_UP_TO_DATE)  assign("This is the most recent version");
    if (id == IDS_VERSION_OUT_OF_DATE) assign("There is a newer version available at www.digicel.net/download");
    if (id == IDS_PROMPT_KEEP_FRAME)   assign("Frame is Modified,\nKeep Changes ?");
    if (id == IDS_NOPLAYER)            assign("Unable to Run Player");
    if (id == IDS_CELL_OPENED)         assign("You Cannot Replace an Open Drawing");
    if (id == IDS_PROMPT_KEEP_CLIP)    assign("Do you want to save the data on the clipboard?");
    if (id == IDS_NO_PASTE)            assign("Cannot Paste, Bad Format");
    if (id == IDS_NO_PASTE_RES)        assign("Cannot Paste Resoultion is Different");
    if (id == IDS_PROMPT_KEEP_MODEL)   assign("Model is Modified,\nKeep Changes ?");
    if (id == IDS_EXT_PALS)            assign("Cannot Open Palette File");
    if (id == IDS_EXT_PALDIFF)         assign("Palette File Data Differs");
    if (id == IDS_EXT_MODEL)           assign("Cannot Open Model File");
    if (id == IDS_EXT_WAVE)            assign("Cannot Open Sound File");
    if (id == IDS_EXT_CHECK)           assign("Scene File Format Updated");
    if (id == IDS_PALETTE_SCENE)       assign("This is the Scene Palette and Cannot Be Edited.\nWould You Like to Switch to a Custom Palette?");
    if (id == IDS_PALETTE_EXTERNAL)    assign("External Palettes Cannot Be Edited");
    if (id == IDS_NO_CUT_EDIT)         assign("You Cannot Cut The Cell That Is Being Edited");
    if (id == IDS_LIB_NO_BG)           assign("Cannot Save BG in Library");
    if (id == IDS_LIB_LOAD)            assign("Cannot Load Library");
    if (id == IDS_NO_WORK)             assign("Nothing To Do");
    if (id == IDS_NO_CREATE)           assign("Cannot Create File");
    if (id == IDS_ERR_QT_INIT)         assign("QT Init Error:%d");
    if (id == IDS_ERR_MAYA_INIT)       assign("Maya Init error:%d");
    if (id == IDS_ERR_MAYA_CLOSE)      assign("Maya close error:%d");
    if (id == IDS_ERR_FILE_READ)       assign("Cannot Read File : %d");
    if (id == IDS_ERR_LIB_SAVE)        assign("Cannot Save Library:%d");
    if (id == IDS_ERR_INI_COPY)        assign("Bad ini copy");
    if (id == IDS_ERR_SCAN_SAVE)       assign("Cannot Save Image");
    if (id == IDS_ERR_LICENSE_UNLOCK)  assign("Online Unlock Error:%d, Please Retry");
    if (id == IDS_ERR_BAD_MOUTH)       assign("Bad Mouth:%d");
    if (id == IDS_ERR_FLV_CREATE)      assign("FLVEncoder did not create file");
    if (id == IDS_PALETTE_DEFAULT)     assign("The Default Palette Cannot Be Edited.\nWould You Like to Switch to a Custom Palette?");
    if (id == IDS_NO_LICENSEMGR)       assign("License Manager Malfunction:%d");
    if (id == IDS_NO_CLIENT)           assign("Client is not Recognized");
    if (id == IDS_NO_RENEW)            assign("License Has Been Revoked");
    if (id == IDS_EDUCATIONAL)         assign("Did You Mean educational ?");
    if (id == IDS_PALETTE_PROTECTED)   assign("Palette Is Protected\nProceed Anyway?");
    if (id == IDS_PALETTE_LOCKED)      assign("Palette Cannot Be Modified?");
    if (id == IDS_NO_TEXTURE_FILE)     assign("Cannot Open Texture File");
    if (id == IDS_NO_LIC_PRODUCTS)     assign("No License Manager Products Avalaable");
    if (id == IDS_EXT_SND_LEVELS)      assign("File exceeds Sound Channel Count");
    if (id == IDS_EXT_WAVE_CHANGE)     assign("Changing Sound File to");
    if (id == IDS_USING_OVERLAY)       assign("This image is an overlay, which can only be viewed and composited.\nFlipBook's tools are not designed to edit overlays.");
    if (id == IDS_EXT_MODEL_CHANGE)    assign("Changing Palette File to");
    if (id == IDS_EXT_PAL_CHANGE)      assign("Changing Model File to");
    if (id == IDS_EXT_TEXTURE_CHANGE)  assign("Changing Texture File to");
    if (id == IDS_NOT_FOR_LITE)        assign("This Feature is not available in LITE versions.");
    if (id == IDS_UPDATE_NOW)          assign("Do you want to exit now and install update ?");
    if (id == IDS_CHECK_INTERNET)      assign("Please check your internet connection.");
    if (id == IDS_NO_DIGICEL_CONNECT)  assign("Unable to check for updates.");
    if (id == IDS_TEMPNOW)             assign("Remind Temp Expired");
    if (id == IDS_DIFF2_USB)           assign("USB Key Has Changed");
    if (id == IDS_REVERT_DEMO)         assign("Reverting to Demo Mode");
    if (id == IDS_NO_USB)              assign("USB Key Is Missing");
    if (id == IDS_DIFF_USB)            assign("USB Key Has Changed\nCheck About for Features");
    if (id == IDS_NO_HELP)             assign("Cannot Access Help File");
    if (id == ID_FILE_REVERT)          assign("Revert document with a new name\nRevert");
    if (id == ID_FILE_SAVEAS_MODEL)    assign("Save Cel as Color Model\nSave Model");
    if (id == ID_FILE_IMPAVI)          assign("Import AVI\nImp AVI");
    if (id == ID_FILE_EDIT)            assign("External Edit\nExt Edit");
    if (id == ID_EDIT_OUTSIDE)         assign("Cut Outside\nCrop");
    if (id == ID_FILE_IMPMOV)          assign("Import MOV\nMovie");
    if (id == ID_VIEW_TIMING)          assign("Timing Mode\nTiming");
    if (id == ID_OPT_LIGHTBOX)         assign("LightBox Options");
    if (id == ID_OPT_CUST_TB)          assign("Customize Tool Bar\nCustom ToolBar");
    if (id == ID_EXPORT_MAYA)          assign("Export Maya Timing\nExport Maya");
    if (id == ID_VIEW_SUBPAL)          assign("View Sub Palette");
    if (id == ID_ZOOM_800)             assign("Zoom 800%");
    if (id == ID_APP_REGISTER)         assign("Register\nRegister");
    if (id == ID_EDIT_PASTEREVERSED)   assign("Paste Reversed\nPaste Rev");
    if (id == ID_VIEW_CAMERA)          assign("Show or Hide Camera Dialog\nCamera");
    if (id == ID_ACTION_FLIP)          assign("Flip Image\nFlip");
    if (id == ID_APP_PROPERTIES)       assign("Displays Scene Properties");
    if (id == ID_VIEW_CAMERA_100)      assign("100 % View");
    if (id == ID_VIEW_CAMERA_50)       assign("50 % Camera View");
    if (id == ID_VIEW_CAMERA_25)       assign("25 % Camera View");
    if (id == ID_VIEW_CAMERA_ENLARGE)  assign("Enlarge Camera View");
    if (id == ID_VIEW_CAMERA_GRID)     assign("Enable/Disable Field Chart");
    if (id == ID_EDIT_MODEL)           assign("Edit Model");
    if (id == ID_FILE_IMPWAVE)         assign("Import Sound");
    if (id == ID_VIEW_LAYER)           assign("Select Layer For Edit\nLayer");
    if (id == ID_VIEW_VIDEO)           assign("Show or Hide Video Window\nVideo");
    if (id == ID_SLIDER_START)         assign("Set Start\nStart");
    if (id == ID_SLIDER_STOP)          assign("Set Stop\nStop");
    if (id == ID_TOOL_PENCIL)          assign("Select Pencil Tool\nPencil");
    if (id == ID_TOOL_TRACE)           assign("Select Trace Tool\nTrace");
    if (id == ID_TOOL_BRUSH)           assign("Select Brush Tool\nBrush");
    if (id == ID_TOOL_FILL)            assign("Select Fill Tool\nFill");
    if (id == ID_TOOL_ZOOM)            assign("Select Zoom Tool\nZoom");
    if (id == ID_TOOL_HAND)            assign("Select Pan Tool\nPan");
    if (id == ID_TOOL_EYEDROP)         assign("Select Eyedropper Tool\nEyedropper");
    if (id == ID_TOOL_ERASER)          assign("Select Erasing Mode\nEraser");
    if (id == ID_TOOL_ERASER_BOX)      assign("Select Erasing Mode\nEraser");
    if (id == ID_TOOL_WAND)            assign("Wand");
    if (id == ID_TOOL_CAM_PAN)         assign("Select Pan Tool\nPan");
    if (id == ID_TOOL_CAM_ZOOM)        assign("Select Zoom Tool\nZoom");
    if (id == ID_TOOL_CAM_ROTATE)      assign("Select Rotate Tool\nRotate");
    if (id == ID_TOOL_CAM_BLUR)        assign("Select Blur Tool\nBlur");
    if (id == ID_TOOL_CAM_ALPHA)       assign("Select Alpha Tool\nAlpha");
    if (id == ID_TOOL_CAM_DALL)        assign("Delete All Keys\nDelAll");
    if (id == ID_TOOL_CAM_DTHIS)       assign("Delete THis Key\nDelThis");
    if (id == ID_TOOL_CAM_NEXT)        assign("Next Key\nNxtKey");
    if (id == ID_TOOL_CAM_PREV)        assign("Previous Key\nPrvKey");
    if (id == ID_ACTION_FILL)          assign("Auto Fill\nFill");
    if (id == ID_VIEW_MODE)            assign("Color or Pencil Test\nColor Mode");
    if (id == ID_VIEW_TOOLS)           assign("Show or Hide Tool Box\nToolBox");
    if (id == ID_OPT_SETTING)          assign("Settings Menu");
    if (id == ID_OPT_FG)               assign("Turn the Light Table On or Off\nLight Bulb");
    if (id == ID_OPT_BG)               assign("Show or Hide Background\nBackground");
    if (id == ID_FILE_EXPAVI)          assign("Export AVI");
    if (id == ID_FILE_SCAN)            assign("Scan\nScan");
    if (id == ID_FILE_PUBLISH)         assign("Publish");
    if (id == ID_FILE_EXPSWF)          assign("Export SWF");
    if (id == ID_EDIT_BLANK)           assign("Insert Blank Cel(s)\nBlank");
    if (id == ID_EDIT_CLEARC)          assign("Clear Canvas\nClear");
    if (id == ID_OPT_SHOWTHUMBS)       assign("Show or Hide Thumbnails\nThumbnails");
    if (id == ID_VIEW_SOUND)           assign("Show or Hide Sound Display\nDisplay Sound");
    if (id == ID_FILE_CVTSTRYBRD)      assign("Convert StoryBoard");
    if (id == ID_FILE_EXPORT)          assign("Export BMP");
    if (id == ID_OPT_AUTOCOMP)         assign("Enable or Disbale Automatic Compositing\nAutoComp");
    if (id == ID_FILE_IMPFILE)         assign("Import File");
    if (id == ID_APP_HELP)             assign("Display Help File\nHelp");
    if (id == ID_ZOOM_25)              assign("Zoom 25%");
    if (id == ID_ZOOM_50)              assign("Zoom 50%");
    if (id == ID_ZOOM_100)             assign("Zoom 100%");
    if (id == ID_ZOOM_200)             assign("Zoom 200%");
    if (id == ID_ZOOM_300)             assign("Zoom 300%");
    if (id == ID_ZOOM_400)             assign("Zoom 400%");
    if (id == ID_ZOOM_FIT)             assign("Zoom to Fit");
    if (id == ID_OPT_KEEP)             assign("Turn  AutoSave On or Off\nAuto Save");
    if (id == ID_EDIT_REVERT)          assign("Revert to Disk Image\nRevert");
    if (id == ID_FILE_CAPTURE)         assign("Video Capture\nCapture");
    if (id == ID_TOOL_INTENSITY)       assign("Adjust Intensity\nIntensity");
    if (id == ID_TOOL_RADIUS)          assign("Adjust Radius\nRadius");
    if (id == ID_FILE_RECORD)          assign("Record\nRecord");
    if (id == ID_EDIT_INS)             assign("Insert Cel(s)\nInsert");
    if (id == ID_EDIT_DEL)             assign("Delete Cel(s)\nDelete");
    if (id == ID_VIEW_SHEET)           assign("Show or Hide XSheet\nXSheet");
    if (id == ID_VIEW_FLIP)            assign("Engage or Disengage Flip Sheet\nFlip");
    if (id == ID_VIEW_PALETTE)         assign("Show or Hide Palette\nPalette");
    if (id == ID_VIEW_VCR)             assign("Display or Hide VCR");
    if (id == IDS_CLEARALL)            assign("This will delete ALL key frames in ALL levels.");
    if (id == OPTPLAYER)               assign("Media Player");
    if (id == IDS_TOO_BIG_AVI)         assign("AVI / Movie is Larger than Max Frames, Maximize Scene ?");
    if (id == IDS_NO_SOUND)            assign("No Sound Hardware Available");
    if (id == IDS_NO_WAVE)             assign("Unable To Open Wave File %1");
    if (id == IDS_BIG_AVI)             assign("AVI / Movie is Larger than Scene, Add Frames ?");
    if (id == IDS_OLD_VFW)             assign("VFW Too Old");
    if (id == IDS_BAD_WRITE)           assign("Write Error");
    if (id == IDS_BAD_TWAIN)           assign("Internal error while preparing for TWAIN acquire.");
    if (id == IDS_CAP_CONNECT)         assign("Cannot connect to video capture driver.");
    if (id == IDS_TWAIN_ERROR)         assign("A TWAIN error occurred");
    if (id == IDS_VID_CAP)             assign("Capture Problem 1");
    if (id == IDS_SCENE_CREATE)        assign("Couldn't make scene");
    if (id == IDS_SCENE_LOAD)          assign("Couldn't load scene");
    if (id == IDS_SCENE_SAVE)          assign("Couldn't save scene");
    if (id == IDS_FILE_CREATE)         assign("Couldn't create %1");
    if (id == IDS_FILE_OPEN)           assign("Couldn't open %1");
    if (id == IDS_FILE_READ)           assign("Couldn't read %1");
    if (id == IDS_SCENE_SAVE1)         assign("Demo Version cannot save a release scene");
    if (id == IDS_SCENE_SAVE2)         assign("Release Version can only edit release scenes");
    if (id == IDS_NOT_HIRES)           assign("Too Big (not HiRes)");
    if (id == IDS_MAX_FRAMES)          assign("Cannot Save, Too Many Frames");
    if (id == IDS_REGISTER3)           assign("Bad Code, First Letter of code must be A or W");
    if (id == IDS_REGISTER4)           assign("Code mismatch, second letter must match id code");
    if (id == IDS_REGISTER5)           assign("Check digit failure, re-enter code");
    if (id == IDS_REGISTER6)           assign("Version Mismatch, re-enter code");
    if (id == IDS_SCENE_NEWER)         assign("Scene created by a newer Version of FlipBook");
    if (id == IDS_WIDE_PASTE)          assign("The Paste Area Is Too Wide, Continue with Narrow ?");
    if (id == IDS_REGISTER1)           assign("Invalid Format. Must be 6 Fields separated by dashes");
    if (id == IDS_REGISTER2)           assign("Code does not decrypt properly, ensure it is correct");
    if (id == IDS_DEMO_WARNING)        assign("Release Version reverting to Demo Mode to load Demo Scene");
    if (id == IDS_AREYOUSURE)          assign("Are You Sure ?");
    if (id == IDS_NOTLITE)             assign("Couldn't load scene, it wasn't made with FlipBook Lite");
    if (id == IDS_NOTPT)               assign("Couldn't load scene, it wasn't made with FlipBook Pencil Test");
    if (id == IDS_DEMO)                assign("Demo Mode");
    if (id == IDS_STANDARD)            assign("Studio");
    if (id == IDS_LITE)                assign("Lite");
    if (id == IDS_PT)                  assign("Pencil Test");
    if (id == IDS_PRO)                 assign("Professional");
    if (id == IDS_NOTSTD)              assign("Couldn't load scene, it wasn't made with FlipBook Studio");
    if (id == IDS_NOMEMORY)            assign("Insufficient Memory");
    if (id == IDS_NOWINTAB)            assign("WinTab Not Available");
    if (id == IDS_TOO_MANY_FRAMES)     assign("Too Many Frames");
    if (id == IDS_TOO_MANY_LEVELS)     assign("Too Many Levels");
    if (id == IDS_ERR_FLV_CONVERT)     assign("FLVEncoder did not convert file");
    if (id == IDS_ERR_FLV_ERROR)       assign("FLVEncoder error:%d");
    if (id == IDS_ERR_PROG2)           assign("Program CallBack Error:%d");
    if (id == IDS_ERR_NO_OPEN)         assign("Cannot Open : %s");
    if (id == IDS_CUT_WARNING)         assign("Do You Want to cut the cell");
    if (id == IDS_AVI_IMPORT)          assign("Importing Video Frames");
    if (id == IDS_FRAME_COUNT_EXCEEDED)assign("Maximum Frame Count Exceeded");
    if (id == IDS_AVIDLG_FASTER)       assign("Scene Rate is Faster");
    if (id == IDS_AVIDLG_SLOWER)       assign("Scene Rate is Slower");
    if (id == IDS_AVIDLG_DROP)         assign("Drop Video Frames");
    if (id == IDS_AVIDLG_ADD)          assign("Add Holds");
    if (id == IDS_AVIDLG_SMALLER)      assign("Frame Size is Smaller Than Scene");
    if (id == IDS_AVIDLG_LARGER)       assign("Frame Size is Larger Than Scene");
    if (id == IDS_AVIDLG_GRAY)         assign("Scale To Fit Since Line Art");
    if (id == IDS_NO_TABLET)           assign("No Tablet");
    if (id == IDS_BLUR_KEY)            assign("Blur Key Type");
    if (id == IDS_ALPHA_KEY)           assign("Alpha Key Type");
    if (id == IDS_EXPORTGIF)           assign("Export GIF File");
    if (id == IDS_EXPORT_MOVIE)        assign("Export MOV File");
    if (id == IDS_EMAIL_BAD)           assign("Invalid email addresss.");
    if (id == IDS_EMAIL_MISMATCH)      assign("The email address must be the same.");
    if (id == IDS_NO_EMAIL)            assign("An email address is required.");
    if (id == IDS_NO_NAME)             assign("A name is required");
    if (id == IDS_NO_ID)               assign("A Product ID is required.");
    if (id == IDS_NO_ORDER)            assign("The Order Number is required.");
    if (id == IDS_CLOSING)             assign("This Application will now close\nafter allowing you to save changes");
    if (id == IDS_EXTRACTWAV)          assign("Extract WAV File");
    if (id == IDS_PAL_IMP_LITE)        assign("LITE cannot import complex palettes");
    if (id == IDS_PAL_READ_LITE)       assign("LITE cannot read complex palettes");
    if (id == IDS_NO_INI_WRITE)        assign("Cannot Modify INI File");
    if (id == IDS_EXPORT_FLV)          assign("Export FLV File");
    if (id == IDS_EXPORT_EMBED)        assign("Export Embedded Files");
    if (id == IDS_SAVE_MODEL)          assign("Save Model File");
    if (id == IDS_LOAD_MODEL)          assign("Load Model File");
    if (id == IDS_CREATE_NEW)          assign("Create New Scene");
    if (id == IDS_EXPORTAVI)           assign("Export AVI File");
    if (id == IDS_EXPORTBMP)           assign("Export Still File");
    if (id == IDS_SELECTPLAY)          assign("Select Player");
    if (id == IDS_SELECTAVI)           assign("Select AVI");
    if (id == IDS_PAL_IMPORT)          assign("Import Palette File");
    if (id == IDS_PAL_EXPORT)          assign("Export Palette File");
    if (id == IDS_NS_KEY)              assign("NS Key Type");
    if (id == IDS_EW_KEY)              assign("EW Key Type");
    if (id == IDS_SCALE_KEY)           assign("Scale Key Type");
    if (id == IDS_ROTATE_KEY)          assign("Rotate Key Type");
    if (id == IDS_EXPORT_LIBRARY)      assign("Export Library File");
    if (id == IDS_IMPORT_LIBRARY)      assign("Import Library File");
    if (id == IDS_EXPORT_CAMERA)       assign("Export Camera Move File");
    if (id == IDS_IMPORT_CAMERA)       assign("Import Camera Move File");
    if (id == WINTAB)                  assign("Use Win Tab");
    if (id == BGMIN)                   assign("Background Darkness");
    if (id == PEG_SHOWBG)              assign("Peg Show BG");
    if (id == PEG_SHOWFG)              assign("Peg Show FG");
    if (id == PEG_DEPTH)               assign("Peg Depth");
    if (id == PEG_DENSITY)             assign("Peg Density");
    if (id == PEN_MINRAD)              assign("Pen Min Radius");
    if (id == PEN_MAXRAD)              assign("Pen Max Radius");
    if (id == PNC_MINDEN)              assign("Pencil Min Density");
    if (id == PNC_MAXDEN)              assign("Pencil Max Density");
    if (id == ERS_MINDEN)              assign("Eraser Min Density");
    if (id == ERS_MAXDEN)              assign("Eraser Max Density");
    if (id == SHT_SHOWBG)              assign("Sheet Show BG");
    if (id == FLIP_FEW)                assign("Flip through the stack");
    if (id == TAB_PRIMARY)             assign("Tablet Primary");
    if (id == JPEG_QUAL)               assign("JPEG");
    if (id == DEF_PREVIEW)             assign("Default Preview");
    if (id == DEF_BROADCAST)           assign("Default Broadcast");
    if (id == MIN_PRESSURE)            assign("Tab Min Pressure");
    if (id == TABLET_USED)             assign("Tablet Was Used");
    if (id == LEFT_SCROLL)             assign("Left Scroll Bars");
    if (id == USE_MOUTHS)              assign("Use Mouths");
    if (id == EDIT_SCRUB)              assign("Edit Scrub Cell");
    if (id == SC_EMBED_OUT)            assign("Embed Data Out");
    if (id == SC_EMBED_IN)             assign("Embed Data In");
    if (id == SC_EMBED_TRIM)           assign("Embed Data Trim");
    if (id == REC_FILE)                assign("Record Cap File");
    if (id == REC_VNAME)               assign("Record Vid Device");
    if (id == REC_ANAME)               assign("Record Aud Device");
    if (id == REC_CNAME)               assign("Record Compressor");
    if (id == REC_CAP_AUD)             assign("Record Cap Audio");
    if (id == REC_CAP_CC)              assign("Record Cap CC");
    if (id == REC_PREVIEW)             assign("Record Preview");
    if (id == REC_SIZE)                assign("Record Preallocate Size");
    if (id == REC_ERASE)               assign("Record Erase File At ");
    if (id == REC_TIME)                assign("Record Timer");
    if (id == REC_DELAY)               assign("Record Delay");
    if (id == RECORD_X)                assign("Record X");
    if (id == RECORD_Y)                assign("Record Y");
    if (id == SC_AVI_SCALE)            assign("Scale AVI");
    if (id == REC_GUID)                assign("Record Cap Video");
    if (id == SC_APPLY_FRAME)          assign("Apply Frame Flags");
    if (id == OVERRIDE_FLAGS)          assign("Override Flags");
    if (id == MAX_PRESSURE)            assign("Tab Max Pressure");
    if (id == AUTOKEEP)                assign("AutoSave");
    if (id == LOOPPLAY)                assign("Loop Play");
    if (id == DEF_WIDTH)               assign("Default Width");
    if (id == DEF_HEIGHT)              assign("Default Height");
    if (id == DEF_FRAMES)              assign("Default Frames");
    if (id == DEF_RATE)                assign("Default Rate");
    if (id == DEF_KIND)                assign("Default Kind");
    if (id == DEF_LEVELS)              assign("Default Levels");
    if (id == XSHEET_HIGHLIGHT)        assign("XSheetHighlight");
    if (id == LEFTHAND)                assign("Left Handed");
    if (id == SHOWTHUMBS)              assign("Show Thumbs");
    if (id == CAPTURE_DEVICE)          assign("Capture Device");
    if (id == VIDCAP_X)                assign("VidCap X");
    if (id == VIDCAP_Y)                assign("VidCap Y");
    if (id == VIDCAP_OPTS)             assign("New VidCap Options");
    if (id == SCAN_CROPPEG)            assign("Scan Crop Pegs");
    if (id == VIDCAP_WIDTH)            assign("VidCap Width");
    if (id == VIDCAP_HEIGHT)           assign("VidCap Height");
    if (id == SELECT_FLAGS)            assign("Selection Flags");
    if (id == SIMPLE_CURSOR)           assign("Simple Cursor");
    if (id == EXTEDIT_OPTS)            assign("ExtEdit Options");
    if (id == HARD_ZOOM)               assign("HardZoom");
    if (id == VIDCAP_WHITE)            assign("VidCap White");
    if (id == VIDCAP_GAMMA)            assign("VidCap Gamma");
    if (id == DEF_HOLD)                assign("Default Hold");
    if (id == TOOLSAME)                assign("Tools Same Color");
    if (id == DEF_FACTOR)              assign("Default Factor");
    if (id == CHECKERBG)               assign("CheckerBoardBG");
    if (id == SCANROTATE)              assign("ScanRotation");
    if (id == PRINTOPTS)               assign("PrintOptions");
    if (id == VIDCAP_RED)              assign("VidCap Red");
    if (id == VIDCAP_GREEN)            assign("VidCap Green");
    if (id == VIDCAP_BLUE)             assign("VidCap Blue");
    if (id == VIDCAP_EXTENT)           assign("VidCap Extent");
    if (id == SCAN_FIELD)              assign("Scan Field Size");
    if (id == SCAN_OFFSET)             assign("Scan Offset");
    if (id == SCAN_PEGTOP)             assign("Scan Pegs On  Top");
    if (id == ID_VCR_EXIT)             assign("Exit VCR\nExit VCR");
    if (id == ID_VCR_HOME)             assign("First Frame\nFirst Frame");
    if (id == ID_VCR_BACK)             assign("Play in Reverse\nPlay Reverse");
    if (id == ID_VCR_STOP)             assign("Pause\nPause");
    if (id == ID_VCR_PLAY)             assign("Play Forward\nPlay Forward");
    if (id == ID_VCR_BSTEP)            assign("Step Back\nBack");
    if (id == ID_VCR_FSTEP)            assign("Step Forward\nForward");
    if (id == ID_VCR_LOOP)             assign("Loop Playback\nLoop Playback");
    if (id == ID_RATE_UP)              assign("Increase Rate");
    if (id == ID_RATE_DOWN)            assign("Decrease Rate");
    if (id == IDC_SLD_START)           assign("Trim Start");
    if (id == IDC_SLD_VOLUME)          assign("Adjust Volume");
    if (id == IDC_SLD_RATE)            assign("Adjust Rate");
    if (id == IDC_SLD_LOOP)            assign("Enable / Disable Loop");
    if (id == IDC_SLD_STOP)            assign("Trim Stop");
    if (id == IDW_SLIDER)              assign("Frame Slider\nSlider");
    if (id == ZIDC_SND_INTERNAL)       assign("#Internal Sound\nUnchecking will loose data");
    if (id == IDC_SND_INTERNALX)       assign("#External Sound\nChecking means sound will\nbesaved with file");
    if (id == IDC_SND_ENABLE)          assign("Enable Sound");
    if (id == IDC_SND_WAVE)            assign("File Name of Wave File");
    if (id == IDC_SND_CHANGE)          assign("Press to select a sound file");
    if (id == IDC_SND_VMARK)           assign("Video Mark\nIn Frames");
    if (id == IDC_SND_SMARK)           assign("Audio Mark\nInSeconds");
    if (id == IDC_SND_SNIP)            assign("Sound Snip Size\nIn Frames");
    if (id == IDC_PEN_FREE)            assign("Freeform Drawing");
    if (id == IDC_PEN_LINE)            assign("Line Drawing");
    if (id == IDC_PEN_RECT)            assign("Rectangle");
    if (id == IDC_PEN_ELLIPSE)         assign("Ellipse / Circle");
    if (id == IDC_PEN_CURVE)           assign("Curve");
    if (id == IDC_PAL_CLONE)           assign("Create a New Palette");
    if (id == IDC_CAP_GRAY)            assign("Use this to capture pencil lines and make the paper disappear.");
    if (id == IDC_CAP_KEYED)           assign("This let's you make one of the colors in the image transparent.");
    if (id == IDC_CAP_PREVIOUS)        assign("This will combine the previous image with the current image.");
    if (id == IDC_CAP_KEY_OPT)         assign("Choose which color to make transparent.");
    if (id == IDC_CAP_WHTSLIDE)        assign("Reduce this value to make more shades of white disappear.");
    if (id == IDC_CAP_GAMSLIDE)        assign("Increase this to make the lines darker.");

	return size() != 0;
}

int CString::GetLength() const
{
    return size();
}

#endif // IMPLEMENT_WINDOWS_APIS
