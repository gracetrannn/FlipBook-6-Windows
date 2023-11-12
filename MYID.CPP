#include "stdafx.h"
#ifndef FLIPBOOK_MAC // Mac implementation is in CFC_Events.h.

#pragma pack(1) // Required to ensure correct SMART IOCTL structure setup
#include "myid.h"
#include "mmsystem.h"
#include <winioctl.h>
#include <Iphlpapi.h>
//#include "nb30.h"

CMyId::CMyId()
{
	nVersion = -1;
	m_mac_count = 0;
}

#ifndef GETWINVER_H
#define GETWINVER_H

#define WUNKNOWNSTR	_T("unknown Windows version")

#define W95STR		_T("Windows 95")
#define W95SP1STR	_T("Windows 95 SP1")
#define W95OSR2STR	_T("Windows 95 OSR2")
#define W98STR		_T("Windows 98")
#define W98SP1STR	_T("Windows 98 SP1")
#define W98SESTR	_T("Windows 98 SE")
#define WMESTR		_T("Windows ME")

#define WNT351STR	_T("Windows NT 3.51")
#define WNT4STR		_T("Windows NT 4")
#define W2KSTR		_T("Windows 2000")
#define WXPSTR		_T("Windows XP")
#define WVISTA  	_T("Windows Vista")
#define W7      	_T("Windows 7")

#define WCESTR		_T("Windows CE")


#define WUNKNOWN	0

#define W9XFIRST	1
#define W95			1
#define W95SP1		2
#define W95OSR2		3
#define W98			4
#define W98SP1		5
#define W98SE		6
#define WME			7
#define W9XLAST		99

#define WNTFIRST	101
#define WNT351		101
#define WNT4		102
#define W2K			103
#define WXP			104
#define WXVISTA  	105
#define WX7   		106
#define WNTLAST		199

#define WCEFIRST	201
#define WCE			201
#define WCELAST		299

BOOL GetWinVer(CString& strVersion, int *nVersion);

#endif //GETWINVER_H


int CMyId::GetOsStr(CString &ztrVersion)
{
	if (nVersion < 0)
		GetOs();
	ztrVersion = strVersion;
	return nVersion;
}

DWORD CMyId::GetVolume()
{
	DWORD dwTemp;
	if (!GetVolumeInformation("C:\\",0,0,&dwTemp,0,0,0,0))
		dwTemp = 0;
	return dwTemp;
}

void CMyId::GetDiskNbr(CString &serNbr, DWORD & dwId)
{
	if (nVersion < 0)
		GetOs();
	GetIDESerialNo(serNbr,dwId);
}



void CMyId::GetOs()
{

	MEMORYSTATUS statex;
	GlobalMemoryStatus (&statex);

	m_dwLoad = statex.dwMemoryLoad;
	m_dwTotalPhys = statex.dwTotalPhys;
	m_dwAvailPhys = statex.dwAvailPhys;
	m_dwTotalVirt = statex.dwTotalVirtual;
	m_dwAvailVirt = statex.dwAvailVirtual;


	nVersion = WUNKNOWN;

	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osinfo))
		return;

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMinorVersion = osinfo.dwMinorVersion;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;
	DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;	// Win 95 needs this
	
	if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
	{
		if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
		{
			strVersion = W95STR;
			nVersion = W95;
		}
		else if ((dwMinorVersion < 10) && 
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
		{
			strVersion = W95SP1STR;
			nVersion = W95SP1;
		}
		else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
		{
			strVersion = W95OSR2STR;
			nVersion = W95OSR2;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
		{
			strVersion = W98STR;
			nVersion = W98;
		}
		else if ((dwMinorVersion == 10) && 
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
		{
			strVersion = W98SP1STR;
			nVersion = W98SP1;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
		{
			strVersion = W98SESTR;
			nVersion = W98SE;
		}
		else if (dwMinorVersion == 90)
		{
			strVersion = WMESTR;
			nVersion = WME;
		}
	}
	else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
		{
			strVersion = WNT351STR;
			nVersion = WNT351;
		}
		else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
		{
			strVersion = WNT4STR;
			nVersion = WNT4;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
		{
			strVersion = W2KSTR;
			nVersion = W2K;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
		{
			strVersion = WXPSTR;
			nVersion = WXP;
		}
		else if ((dwMajorVersion == 6) && (dwMinorVersion == 0))
		{
			strVersion = WVISTA;
			nVersion = WXVISTA;
		}
		else if ((dwMajorVersion == 6) && (dwMinorVersion == 1))
		{
			strVersion = W7;
			nVersion = WX7;
		}
	}
//	else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
//	{
//		strVersion = WCESTR;
//		nVersion = WCE;
//	}
	
}


   //  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  4
#define  IDENTIFY_BUFFER_SIZE  512


   //  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition



   //  GETVERSIONOUTPARAMS contains the data returned from the 
   //  Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
   BYTE bVersion;      // Binary driver version.
   BYTE bRevision;     // Binary driver revision.
   BYTE bReserved;     // Not used.
   BYTE bIDEDeviceMap; // Bit map of IDE devices.
   DWORD fCapabilities; // Bit mask of driver capabilities.
   DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;


   //  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS 
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported


   //  IDE registers
#ifndef CAP_SMART_CMD
typedef struct _IDEREGS
{
   BYTE bFeaturesReg;       // Used for specifying SMART "commands".
   BYTE bSectorCountReg;    // IDE sector count register
   BYTE bSectorNumberReg;   // IDE sector number register
   BYTE bCylLowReg;         // IDE low order cylinder value
   BYTE bCylHighReg;        // IDE high order cylinder value
   BYTE bDriveHeadReg;      // IDE drive/head register
   BYTE bCommandReg;        // Actual IDE command.
   BYTE bReserved;          // reserved for future use.  Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;
#endif
   //  SENDCMDINPARAMS contains the input parameters for the 
   //  Send Command to Drive function.
//#if !(_WIN32_WINNT >= 0x0400)
#ifndef SMART_CYL_HI
typedef struct _SENDCMDINPARAMS
{
   DWORD     cBufferSize;   //  Buffer size in bytes
   IDEREGS   irDriveRegs;   //  Structure with drive register values.
   BYTE bDriveNumber;       //  Physical drive number to send 
                            //  command to (0,1,2,3).
   BYTE bReserved[3];       //  Reserved for future expansion.
   DWORD     dwReserved[4]; //  For future use.
   BYTE      bBuffer[1];    //  Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;
#endif

   //  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.


   // Status returned from driver
#ifndef CAP_SMART_CMD
typedef struct _DRIVERSTATUS
{
   BYTE  bDriverError;  //  Error code from driver, or 0 if no error.
   BYTE  bIDEStatus;    //  Contents of IDE Error register.
                        //  Only valid when bDriverError is SMART_IDE_ERROR.
   BYTE  bReserved[2];  //  Reserved for future expansion.
   DWORD  dwReserved[2];  //  Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;


   // Structure returned by PhysicalDrive IOCTL for several commands

typedef struct _SENDCMDOUTPARAMS
{
   DWORD         cBufferSize;   //  Size of bBuffer in bytes
   DRIVERSTATUS  DriverStatus;  //  Driver status structure.
   BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
#endif

   // The following struct defines the interesting part of the IDENTIFY
   // buffer:
typedef struct _IDSECTOR
{
   USHORT  wGenConfig;
   USHORT  wNumCyls;
   USHORT  wReserved;
   USHORT  wNumHeads;
   USHORT  wBytesPerTrack;
   USHORT  wBytesPerSector;
   USHORT  wSectorsPerTrack;
   USHORT  wVendorUnique[3];
   CHAR    sSerialNumber[20];
   USHORT  wBufferType;
   USHORT  wBufferSize;
   USHORT  wECCSize;
   CHAR    sFirmwareRev[8];
   CHAR    sModelNumber[40];
   USHORT  wMoreVendorUnique;
   USHORT  wDoubleWordIO;
   USHORT  wCapabilities;
   USHORT  wReserved1;
   USHORT  wPIOTiming;
   USHORT  wDMATiming;
   USHORT  wBS;
   USHORT  wNumCurrentCyls;
   USHORT  wNumCurrentHeads;
   USHORT  wNumCurrentSectorsPerTrack;
   ULONG   ulCurrentSectorCapacity;
   USHORT  wMultSectorStuff;
   ULONG   ulTotalAddressableSectors;
   USHORT  wSingleWordDMA;
   USHORT  wMultiWordDMA;
   BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;


typedef struct _SRB_IO_CONTROL
{
   ULONG HeaderLength;
   UCHAR Signature[8];
   ULONG Timeout;
   ULONG ControlCode;
   ULONG ReturnCode;
   ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;


   // Define global buffers.
BYTE IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];


char *ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex);
void PrintIdeInfo (int drive, DWORD diskdata [256]);
BOOL DoIDENTIFY (HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE,
                 PDWORD);


int ReadPhysicalDriveInNT (void)
{
   int done = FALSE;
   int drive = 0;

//   for (drive = 0; !done && drive < MAX_IDE_DRIVES; drive++)
//   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      char driveName [256];

      sprintf (driveName, "\\\\.\\PhysicalDrive%d", drive);

         //  Windows NT, Windows 2000, must have admin rights
      hPhysicalDriveIOCTL = CreateFile (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
		{
		int z;
		for (z = 0; z < 2; z++)
			{
            BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
         	DWORD               cbBytesReturned = 0;
            SENDCMDINPARAMS  scip;
            bIDCmd = !z ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

            memset (&scip, 0, sizeof(scip));
            memset (IdOutCmd, 0, sizeof(IdOutCmd));

            if ( DoIDENTIFY (hPhysicalDriveIOCTL, 
                       &scip, 
                       (PSENDCMDOUTPARAMS)&IdOutCmd, 
                       (BYTE) bIDCmd,
                       (BYTE) drive,
                       &cbBytesReturned))
            	{
               	DWORD diskdata [256];
               int ijk = 0;
               USHORT *pIdSector = (USHORT *)
                             ((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;

               for (ijk = 0; ijk < 256; ijk++)
                  diskdata [ijk] = pIdSector [ijk];

               PrintIdeInfo (drive, diskdata);

               done = TRUE;
				break;
			}
            }
	    }

         CloseHandle (hPhysicalDriveIOCTL);
      }
   

   return done;
}


   // DoIDENTIFY
   // FUNCTION: Send an IDENTIFY command to the drive
   // bDriveNum = 0-3
   // bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
BOOL DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                 PDWORD lpcbBytesReturned)
{
      // Set up data structures for IDENTIFY command.
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
   pSCIP -> irDriveRegs.bFeaturesReg = 0;
   pSCIP -> irDriveRegs.bSectorCountReg = 1;
   pSCIP -> irDriveRegs.bSectorNumberReg = 1;
   pSCIP -> irDriveRegs.bCylLowReg = 0;
   pSCIP -> irDriveRegs.bCylHighReg = 0;

      // Compute the drive number.
   pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

      // The command can either be IDE identify or ATAPI identify.
   pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
   pSCIP -> bDriveNumber = bDriveNum;
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

   return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
               (LPVOID) pSCIP,
               sizeof(SENDCMDINPARAMS) - 1,
               (LPVOID) pSCOP,
               sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               lpcbBytesReturned, NULL) );
}



// ------------------------------------------------- //
//                   WinIo v1.2                      //
//  Direct Hardware Access Under Windows 9x/NT/2000  //
//        Copyright 1998-2000 Yariv Kaplan           //
//           http://www.internals.com                //
// ------------------------------------------------- //

//#include <windows.h>
//#include "instdrv.h"

BOOL LoadDeviceDriver( const TCHAR * Name, const TCHAR * Path, HANDLE * lphDevice );
BOOL UnloadDeviceDriver( const TCHAR * Name );

HANDLE hDriver;
bool IsNT;
bool IsWinIoInitialized = false;


bool IsWinNT()
{
  OSVERSIONINFO OSVersionInfo;

  OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&OSVersionInfo);

  return OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;
}


bool InitializeWinIo()
{
  char szExePath[MAX_PATH];
  PSTR pszSlash;

  IsNT = IsWinNT();

  if (IsNT)
  {
    if (!GetModuleFileName(GetModuleHandle(NULL), szExePath, sizeof(szExePath)))
      return false;

    pszSlash = strrchr(szExePath, '\\');

    if (pszSlash)
      pszSlash[1] = 0;
    else
      return false;

    strcat(szExePath, "winio.sys");

//    UnloadDeviceDriver("WINIO");

//    if (!LoadDeviceDriver("WINIO", szExePath, &hDriver))
//      return false;
  }
  
  IsWinIoInitialized = true;

  return true;
}


void ShutdownWinIo()
{
//  if (IsNT)
//    UnloadDeviceDriver("WINIO");
}


// ------------------------------------------------ //
//                  Port32 v3.0                     //
//    Direct Port Access Under Windows 9x/NT/2000   //
//        Copyright 1998-2000 Yariv Kaplan          //
//            http://www.internals.com              //
// ------------------------------------------------ //

//#include <windows.h>
#include <winioctl.h>
#include "port32.h"
#include "winio.h"
//#include "general.h"

// These are our ring 0 functions responsible for tinkering with the hardware ports.
// They have a similar privilege to a Windows VxD and are therefore free to access
// protected system resources (such as the page tables) and even place calls to
// exported VxD services.

__declspec(naked) void Ring0GetPortVal()
{
  _asm
  {
    Cmp CL, 1
    Je ByteVal
    Cmp CL, 2
    Je WordVal
    Cmp CL, 4
    Je DWordVal

ByteVal:

    In AL, DX
    Mov [EBX], AL
    Retf

WordVal:

    In AX, DX
    Mov [EBX], AX
    Retf

DWordVal:

    In EAX, DX
    Mov [EBX], EAX
    Retf
  }
}


__declspec(naked) void Ring0SetPortVal()
{
  _asm
  {
    Cmp CL, 1
    Je ByteVal
    Cmp CL, 2
    Je WordVal
    Cmp CL, 4
    Je DWordVal

ByteVal:

    Mov AL, [EBX]
    Out DX, AL
    Retf

WordVal:

    Mov AX, [EBX]
    Out DX, AX
    Retf

DWordVal:

    Mov EAX, [EBX]
    Out DX, EAX
    Retf
  }
}


// This function makes it possible to call ring 0 code from a ring 3
// application.

bool CallRing0(PVOID pvRing0FuncAddr, WORD wPortAddr, PDWORD pdwPortVal, BYTE bSize)
{

  struct GDT_DESCRIPTOR *pGDTDescriptor;
  struct GDTR gdtr;
  WORD CallgateAddr[3];
  WORD wGDTIndex = 1;

  _asm Sgdt [gdtr]

  // Skip the null descriptor

  pGDTDescriptor = (struct GDT_DESCRIPTOR *)(gdtr.dwGDTBase + 8);

  // Search for a free GDT descriptor

  for (wGDTIndex = 1; wGDTIndex < (gdtr.wGDTLimit / 8); wGDTIndex++)
  {
    if (pGDTDescriptor->Type == 0     &&
        pGDTDescriptor->System == 0   &&
        pGDTDescriptor->DPL == 0      &&
        pGDTDescriptor->Present == 0)
    {
      // Found one !
      // Now we need to transform this descriptor into a callgate.
      // Note that we're using selector 0x28 since it corresponds
      // to a ring 0 segment which spans the entire linear address
      // space of the processor (0-4GB).

      struct CALLGATE_DESCRIPTOR *pCallgate;

      pCallgate =	(struct CALLGATE_DESCRIPTOR *) pGDTDescriptor;
      pCallgate->Offset_0_15 = LOWORD(pvRing0FuncAddr);
      pCallgate->Selector = 0x28;
      pCallgate->ParamCount =	0;
      pCallgate->Unused = 0;
      pCallgate->Type = 0xc;
      pCallgate->System = 0;
      pCallgate->DPL = 3;
      pCallgate->Present = 1;
      pCallgate->Offset_16_31 = HIWORD(pvRing0FuncAddr);

      // Prepare the far call parameters

      CallgateAddr[0] = 0x0;
      CallgateAddr[1] = 0x0;
      CallgateAddr[2] = (wGDTIndex << 3) | 3;

      // Please fasten your seat belts!
      // We're about to make a hyperspace jump into RING 0.

      _asm Mov DX, [wPortAddr]
      _asm Mov EBX, [pdwPortVal]
      _asm Mov CL, [bSize]
      _asm Call FWORD PTR [CallgateAddr]

      // We have made it !
      // Now free the GDT descriptor

      memset(pGDTDescriptor, 0, 8);

      // Our journey was successful. Seeya.

      return true;
    }

    // Advance to the next GDT descriptor

    pGDTDescriptor++; 
  }

  // Whoops, the GDT is full

  return false;
}


bool GetPortVal(WORD wPortAddr, PDWORD pdwPortVal, BYTE bSize)
{
  bool Result;
  DWORD dwBytesReturned;
  struct tagPort32Struct Port32Struct;

  if (IsNT)
  {
    if (!IsWinIoInitialized)
      return false;

    Port32Struct.wPortAddr = wPortAddr;
    Port32Struct.bSize = bSize;

    if (!DeviceIoControl(hDriver, IOCTL_WINIO_READPORT, &Port32Struct,
                         sizeof(struct tagPort32Struct), &Port32Struct, 
						 sizeof(struct tagPort32Struct),
                         &dwBytesReturned, NULL))
      return false;
    else
      *pdwPortVal = Port32Struct.dwPortVal;
  }
  else
  {
    Result = CallRing0((PVOID)Ring0GetPortVal, wPortAddr, pdwPortVal, bSize);

    if (Result == false)
      return false;
  }

  return true;
}


bool SetPortVal(WORD wPortAddr, DWORD dwPortVal, BYTE bSize)
{
  DWORD dwBytesReturned;
  struct tagPort32Struct Port32Struct;

  if (IsNT)
  {
    if (!IsWinIoInitialized)
      return false;

    Port32Struct.wPortAddr = wPortAddr;
    Port32Struct.dwPortVal = dwPortVal;
    Port32Struct.bSize = bSize;

    if (!DeviceIoControl(hDriver, IOCTL_WINIO_WRITEPORT, &Port32Struct,
                         sizeof(struct tagPort32Struct), NULL, 0, &dwBytesReturned, NULL))
      return false;
  }
  else
    return CallRing0((PVOID)Ring0SetPortVal, wPortAddr, &dwPortVal, bSize);
  
  return true;
}


int ReadDrivePortsInWin9X (void)
{
   int done = FALSE;
   int drive = 0;

   InitializeWinIo ();   

      //  Get IDE Drive info from the hardware ports
      //  loop thru all possible drives
   for (drive = 0; drive < 4; drive++)
   {
      DWORD diskdata [256];
      WORD  baseAddress = 0;   //  Base address of drive controller 
      DWORD portValue = 0;
      int waitLoop = 0;
      int index = 0;
 
      switch (drive / 2)
      {
         case 0: baseAddress = 0x1f0; break;
         case 1: baseAddress = 0x170; break;
         case 2: baseAddress = 0x1e8; break;
         case 3: baseAddress = 0x168; break;
      }

         //  Wait for controller not busy 
      waitLoop = 100000;
      while (--waitLoop > 0)
      {
         GetPortVal ((WORD) (baseAddress + 7), &portValue, (BYTE) 1);
            //  drive is ready
         if ((portValue & 0x40) == 0x40) break;
            //  previous drive command ended in error
         if ((portValue & 0x01) == 0x01) break;
      }

      if (waitLoop < 1) continue;

         //  Set Master or Slave drive
      if ((drive % 2) == 0)
         SetPortVal ((WORD) (baseAddress + 6), 0xA0, 1);
      else
         SetPortVal ((WORD) (baseAddress + 6), 0xB0, 1);

         //  Get drive info data
      SetPortVal ((WORD) (baseAddress + 7), 0xEC, 1);

         // Wait for data ready 
      waitLoop = 100000;
      while (--waitLoop > 0)
      {
         GetPortVal ((WORD) (baseAddress + 7), &portValue, 1);
            //  see if the drive is ready and has it's info ready for us
         if ((portValue & 0x48) == 0x48) break;
            //  see if there is a drive error
         if ((portValue & 0x01) == 0x01) break;
      }

         //  check for time out or other error                                                    
      if (waitLoop < 1 || portValue & 0x01) continue;

         //  read drive id information
      for (index = 0; index < 256; index++)
      {
         diskdata [index] = 0;   //  init the space
         GetPortVal (baseAddress, &(diskdata [index]), 2);
      }

      PrintIdeInfo (drive, diskdata);
      done = TRUE;
		break;
   }

   ShutdownWinIo ();

   return done;
}


#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE


int ReadIdeDriveAsScsiDriveInNT (void)
{
   int done = FALSE;
   int controller = 0;

   for (controller = 0; controller < 2; controller++)
   {
      HANDLE hScsiDriveIOCTL = 0;
      char   driveName [256];

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      sprintf (driveName, "\\\\.\\Scsi%d:", controller);

         //  Windows NT, Windows 2000, any rights should do
      hScsiDriveIOCTL = CreateFile (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      // if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE)
      //    printf ("Unable to open SCSI controller %d, error code: 0x%lX\n",
      //            controller, GetLastError ());

      if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
      {
         int drive = 0;

         for (drive = 0; drive < 2; drive++)
         {
            char buffer [sizeof (SRB_IO_CONTROL) + SENDIDLENGTH];
            SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer;
            SENDCMDINPARAMS *pin =
                   (SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
            DWORD dummy;
   
            memset (buffer, 0, sizeof (buffer));
            p -> HeaderLength = sizeof (SRB_IO_CONTROL);
            p -> Timeout = 10000;
            p -> Length = SENDIDLENGTH;
            p -> ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
            strncpy ((char *) p -> Signature, "SCSIDISK", 8);
  
            pin -> irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
            pin -> bDriveNumber = drive;

            if (DeviceIoControl (hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) +
                                         sizeof (SENDCMDINPARAMS) - 1,
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) + SENDIDLENGTH,
                                 &dummy, NULL))
            {
               PSENDCMDOUTPARAMS pOut =
                    (PSENDCMDOUTPARAMS) (buffer + sizeof (SRB_IO_CONTROL));
               IDSECTOR *pId = (IDSECTOR *) (pOut -> bBuffer);
               if (pId -> sModelNumber [0])
               {
                  DWORD diskdata [256];
                  int ijk = 0;
                  USHORT *pIdSector = (USHORT *) pId;
          
                  for (ijk = 0; ijk < 256; ijk++)
                     diskdata [ijk] = pIdSector [ijk];

                  PrintIdeInfo (controller * 2 + drive, diskdata);

                  done = TRUE;
               }
            }
         }
         CloseHandle (hScsiDriveIOCTL);
      }
   }

   return done;
}


char HardDriveSerialNumber [1024];


void PrintIdeInfo (int drive, DWORD diskdata [256])
{
      //  copy the hard driver serial number to the buffer
   strcpy (HardDriveSerialNumber, ConvertToString (diskdata, 10, 19));

#ifdef PRINTING_TO_CONSOLE_ALLOWED

   switch (drive / 2)
   {
      case 0: printf ("\nPrimary Controller - ");
              break;
      case 1: printf ("\nSecondary Controller - ");
              break;
      case 2: printf ("\nTertiary Controller - ");
              break;
      case 3: printf ("\nQuaternary Controller - ");
              break;
   }

   switch (drive % 2)
   {
      case 0: printf ("Master drive\n\n");
              break;
      case 1: printf ("Slave drive\n\n");
              break;
   }

   printf ("Drive Model Number________________: %s\n",
           ConvertToString (diskdata, 27, 46));
   printf ("Drive Serial Number_______________: %s\n",
           ConvertToString (diskdata, 10, 19));
   printf ("Drive Controller Revision Number__: %s\n",
           ConvertToString (diskdata, 23, 26));

   printf ("Controller Buffer Size on Drive___: %u bytes\n",
           diskdata [21] * 512);

   printf ("Drive Type________________________: ");
   if (diskdata [0] & 0x0080)
      printf ("Removable\n");
   else if (diskdata [0] & 0x0040)
      printf ("Fixed\n");
   else printf ("Unknown\n");
           
   printf ("Physical Geometry:     "
           "%u Cylinders     %u Heads     %u Sectors per track\n",
           diskdata [1], diskdata [3], diskdata [6]);

#else   //  PRINTING_TO_CONSOLE_ALLOWED

   //  nothing to do

#endif  // PRINTING_TO_CONSOLE_ALLOWED

}


char *ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex)
{
   static char string [1024];
   int index = 0;
   int position = 0;

      //  each integer has two characters stored in it backwards
   for (index = firstIndex; index <= lastIndex; index++)
   {
         //  get high byte for 1st character
      string [position] = (char) (diskdata [index] / 256);
      position++;

         //  get low byte for 2nd character
      string [position] = (char) (diskdata [index] % 256);
      position++;
   }

      //  end the string 
   string [position] = '\0';

      //  cut off the trailing blanks
   for (index = position - 1; index > 0 && ' ' == string [index]; index--)
      string [index] = '\0';

   return string;
}


DWORD getHardDriveComputerID ()
{
   int done = FALSE;
   //char string [1024];
   __int64 id = 0;

   strcpy (HardDriveSerialNumber, "");

      //  this works under WinNT4 or Win2K if you have admin rights
   done = ReadPhysicalDriveInNT ();

      //  this should work in WinNT or Win2K if previous did not work
      //  this is kind of a backdoor via the SCSI mini port driver into
      //     the IDE drives
   if ( ! done) done = ReadIdeDriveAsScsiDriveInNT ();

      //  this works under Win9X and calls WINIO.DLL
   if ( ! done) done = ReadDrivePortsInWin9X ();

   if (done)
   {
      char *p = HardDriveSerialNumber;

      //WriteConstantString ("HardDriveSerialNumber", HardDriveSerialNumber);

         //  ignore first 5 characters from western digital hard drives if
         //  the first four characters are WD-W
      if ( ! strncmp (HardDriveSerialNumber, "WD-W", 4)) p += 5;
      for ( ; p && *p; p++)
      {
         if ('-' == *p) continue;
         id *= 10;
         switch (*p)
         {
            case '0': id += 0; break;
            case '1': id += 1; break;
            case '2': id += 2; break;
            case '3': id += 3; break;
            case '4': id += 4; break;
            case '5': id += 5; break;
            case '6': id += 6; break;
            case '7': id += 7; break;
            case '8': id += 8; break;
            case '9': id += 9; break;
            case 'a': case 'A': id += 10; break;
            case 'b': case 'B': id += 11; break;
            case 'c': case 'C': id += 12; break;
            case 'd': case 'D': id += 13; break;
            case 'e': case 'E': id += 14; break;
            case 'f': case 'F': id += 15; break;
            case 'g': case 'G': id += 16; break;
            case 'h': case 'H': id += 17; break;
            case 'i': case 'I': id += 18; break;
            case 'j': case 'J': id += 19; break;
            case 'k': case 'K': id += 20; break;
            case 'l': case 'L': id += 21; break;
            case 'm': case 'M': id += 22; break;
            case 'n': case 'N': id += 23; break;
            case 'o': case 'O': id += 24; break;
            case 'p': case 'P': id += 25; break;
            case 'q': case 'Q': id += 26; break;
            case 'r': case 'R': id += 27; break;
            case 's': case 'S': id += 28; break;
            case 't': case 'T': id += 29; break;
            case 'u': case 'U': id += 30; break;
            case 'v': case 'V': id += 31; break;
            case 'w': case 'W': id += 32; break;
            case 'x': case 'X': id += 33; break;
            case 'y': case 'Y': id += 34; break;
            case 'z': case 'Z': id += 35; break;
         }                            
      }
   }

      //   make sure no bigger than 16^7
//   if (id > 268435455) id %= 268435456;

#ifdef PRINTING_TO_CONSOLE_ALLOWED

   printf ("\nComputer ID_______________________: %d\n", id);

#endif

   return (DWORD) id;
}


bool CMyId::GetIDESerialNo(CString & csSerialNumber, DWORD & dwId)
{
	int i;
	DWORD prev = -1;
	DWORD id;
	for (i = 0; i < 10; i++)
		{
   		id = getHardDriveComputerID ();
		if (id)
			{
			if (id == prev)
				break;
			prev = id;
			}
		}
	dwId = id;
	csSerialNumber = HardDriveSerialNumber;
	return 0;
}


// this is pretty straight forward. as long as it's a fairly-recent processor, we'll
// try to get the speed. first, though, we set the priority to high ... so that
// we'll get a greater chance of getting the correct speed.
// the current algorithm just goes until it gets two results in a row that match.
// a better approach may be to take two or three readings and average them.
DWORD CMyId::determineCpuSpeed(void)
{
   const int MAX_TRIES = 10;
   DWORD dwSpeed = 0;
   DWORD dwTestValue = UINT_MAX;
   int   nNumTimes = 0;

 //  if ( ((m_cpuStats.getVendor() == CpuStats::VENDOR_INTEL) && (m_cpuStats.getFamily() > 4)) ||
//        ((m_cpuStats.getVendor() == CpuStats::VENDOR_AMD)   && (m_cpuStats.getFamily() > 5)) )
   {
      DWORD dwStartingPriority = GetPriorityClass(GetCurrentProcess());
      SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

      //--------------------------------------------------------
      // Find the first two similarities up to ten times.
      // This should be a maximum of five seconds.
      // 
/*
      while ((dwSpeed != dwTestValue) && (nNumTimes < MAX_TRIES))
      {
         dwTestValue = dwSpeed;
         dwSpeed = calculateCpuSpeed();
         nNumTimes++;
      }	
*/
	int i;
#define XCOUNT 3
	for (i = 0; i < XCOUNT; i++)
		dwSpeed += calculateCpuSpeed();
	
      SetPriorityClass(GetCurrentProcess(), dwStartingPriority);
	dwSpeed /= i;
	return dwSpeed+8;
      //--------------------------------------------------------
      // If we've done ten queries and STILL don't have a 
      // valid reading, don't change anything so the speed will
      // be Unknown. This if below should almost always evauluate
      // to TRUE.
      //
      if (nNumTimes != MAX_TRIES)
      {
		return dwSpeed;
      }
   }
	return 0;
}


// this function is ripped from amd
DWORD CMyId::calculateCpuSpeed(void) const
{
   int   nTimeStart = 0;
   int   nTimeStop = 0;
   DWORD dwStartTicks = 0;
   DWORD dwEndTicks = 0;
   DWORD dwTotalTicks = 0;
   DWORD dwCpuSpeed = 0;

   nTimeStart = timeGetTime();

   for(;;)
   {
      nTimeStop = timeGetTime();

      if ((nTimeStop - nTimeStart) > 1)
      {
         dwStartTicks = determineTimeStamp();
         break;
      }
   }

   nTimeStart = nTimeStop;

   for(;;)
   {
      nTimeStop = timeGetTime();
      if ((nTimeStop - nTimeStart) > 500)    // one-half second
      {
         dwEndTicks = determineTimeStamp();
         break;
      }
   }

   dwTotalTicks = dwEndTicks - dwStartTicks;
   dwCpuSpeed = dwTotalTicks / 500000;

   return (dwCpuSpeed);
}

// stolen from amd
DWORD CMyId::determineTimeStamp(void) const
{
   DWORD dwTickVal;

   __asm
   {
      _emit 0Fh
      _emit 31h
      mov   dwTickVal, eax
   }

   return (dwTickVal);
}

void CMyId::GetMac(LPSTR pMac)
{
	memset(pMac, 0, 6);
	DWORD * pz = (DWORD *)(pMac+2);
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	ASSERT(dwStatus == ERROR_SUCCESS);			// Verify return value is valid, no buffer overflow
	if (dwStatus != ERROR_SUCCESS)
		return;
	
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
	do {
		unsigned char * p = pAdapterInfo->Address;
		if( p[0] != '0x44' && p[1] != '0x45' && p[2] != '0x53' &&
				p[3] != '0x54' && p[4] != '\0' && p[5] != '\0' )
			{
			LPSTR pc = (LPSTR)pMac;
			for (int j = 0;j<6; j++)
				pc[j] = p[j];
			break;
			}
		pAdapterInfo = pAdapterInfo->Next;		// Progress through linked list
		}
	while(pAdapterInfo);						// Terminate if last adapter
}

/*
	obtains up to four mac addresses into m_macs

*/
void CMyId::NewMac()
{
	m_mac_count = 0;
//m_macs[m_mac_count++] = 0x12345678; // for tetsing
//m_macs[m_mac_count++] = 0x87654321;
//m_macs[m_mac_count++] = 0xdeadbeef;
//m_macs[m_mac_count++] = 0xcafe1234;
//	m_macs[m_mac_count++] = 0x9e723c47;//0x473c729e;
//	m_macs[m_mac_count++] = 0x89c92514;//1425c989;
//	m_macs[m_mac_count++] = 0x9f723c47;//0x473c729e;
//	m_macs[m_mac_count++] = 0x71265678;
//	m_macs[m_mac_count++] = 0x26201234;
//	return;
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	ASSERT(dwStatus == ERROR_SUCCESS);			// Verify return value is valid, no buffer overflow
	if (dwStatus != ERROR_SUCCESS)
		return;
	
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
	do {
		unsigned char * p = pAdapterInfo->Address;
		BOOL bGood = TRUE;
		if( p[0] == 0x44 && p[1] == 0x45 && p[2] == 0x53 &&
					p[3] == 0x54 && p[4] == 0)
			bGood = 0;
		if (!p[0] && !p[1] && !p[2] && !p[3])
				bGood = 0;
		if (bGood)
			{
			DWORD * pz = (DWORD *)(p+2);
			m_macs[m_mac_count++] = SWAPV(*pz);
			if(m_mac_count >= 4)
				break;
			}
		pAdapterInfo = pAdapterInfo->Next;		// Progress through linked list
		}
	while(pAdapterInfo);	// Terminate if last adapter
//	m_mac_count--;
//	m_macs[m_mac_count++] = 0x56781234; // for testing
//	m_mac_count = 0;
}
/*
	used when registering to return a combination existing mac addresses
	into a single 32 bit value
	we use the high order bits because on PC these are the least
	signifcant bits of mac address
*/
UINT CMyId::GetNewMac( DWORD  & umac) // returns count
{
	NewMac();
	UINT mask, shift;
	UINT v,i;
	if (m_mac_count == 4)
		shift = 8;
	else if (m_mac_count == 3)
		shift = 10;
	else if (m_mac_count == 2)
		shift = 16;
	else
		shift = 0;
	if (shift)
		{
		mask = 1;
		mask <<= shift;
		mask--;
		mask <<= (32 - shift); // 0xff000000
		}
	else
		mask = 0xffffffff;
	v = 0;
	for (i = 0; i < m_mac_count; i++)
		{
		v >>= shift;
		v |= (m_macs[i] & mask);// combine all mac addresses into one 32 bit
		}
	umac = v;
	return m_mac_count > 1 ? m_mac_count + 2 : m_mac_count;
//	if (m_mac_count > 1)
//		m_mac_count += 2;
//	return m_mac_count ;		//major bug 8/28/10
}

/*
	this searches all existing mac addresses to see if any match 
	one of the ones originally found
*/
BOOL CMyId::ChkNewMac( UINT  umac, UINT count)
{
	NewMac();
	UINT mask, shift;
	UINT i,j,v;
	if (count == 4)
		shift = 8;
	else if (count == 3)
		shift = 10;
	else if (count == 2)
		shift = 16;
	else
		shift = 0;
	if (shift)
		{
		mask = 1;
		mask <<= shift;
		mask--;
		mask <<= (32 - shift); // 0xff000000
		}
	else
		mask = 0xffffffff;
	v = umac;
	for (j = 0; j < count; j++)
		{
		for (i = 0; i < m_mac_count; i++)
			{
			if ((m_macs[i] & mask) == (v & mask))
				break;
			}
		if (i < m_mac_count)
			break;
		v <<= shift;
		}
	return (i < m_mac_count) ? 1 : 0;
}

#endif
