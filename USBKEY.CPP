#include "stdafx.h"
#ifndef FLIPBOOK_MAC
#include "usbkey.h"
#include <basetyps.h>
#include <winioctl.h>
#include <string.h>
#include <usbioctl.h>

typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE *Next;
    UCHAR                           DescriptorIndex;
    USHORT                          LanguageID;
    USB_STRING_DESCRIPTOR           StringDescriptor[1];
//
//	originally a null array, rick got tired of warnings
//	so he added an element, he thinks since hey are linked
//	 size is irrelevent
//
} STRING_DESCRIPTOR_NODE, *PSTRING_DESCRIPTOR_NODE;


typedef struct
{
    PUSB_NODE_INFORMATION               HubInfo;        // NULL if not a HUB

    PCHAR                               HubName;        // NULL if not a HUB

    PUSB_NODE_CONNECTION_INFORMATION    ConnectionInfo; // NULL if root HUB

    PUSB_DESCRIPTOR_REQUEST             ConfigDesc;     // NULL if root HUB

    PSTRING_DESCRIPTOR_NODE             StringDescs;

} USBDEVICEINFO, *PUSBDEVICEINFO;


typedef struct _USB_INTERFACE_DESCRIPTOR2 {
    UCHAR  bLength;             // offset 0, size 1
    UCHAR  bDescriptorType;     // offset 1, size 1
    UCHAR  bInterfaceNumber;    // offset 2, size 1
    UCHAR  bAlternateSetting;   // offset 3, size 1
    UCHAR  bNumEndpoints;       // offset 4, size 1
    UCHAR  bInterfaceClass;     // offset 5, size 1
    UCHAR  bInterfaceSubClass;  // offset 6, size 1
    UCHAR  bInterfaceProtocol;  // offset 7, size 1
    UCHAR  iInterface;          // offset 8, size 1
    USHORT wNumClasses;         // offset 9, size 2
} USB_INTERFACE_DESCRIPTOR2, *PUSB_INTERFACE_DESCRIPTOR2;

//*****************************************************************************
// D E F I N E S
//*****************************************************************************

#define NUM_HCS_TO_CHECK 10
#if _zDEBUG

#define ALLOC(dwBytes) MyAlloc(__FILE__, __LINE__, (dwBytes))

#define REALLOC(hMem, dwBytes) MyReAlloc((hMem), (dwBytes))

#define FREE(hMem)  MyFree((hMem))

#define CHECKFORLEAKS() MyCheckForLeaks()

#define OOPS() Oops(__FILE__, __LINE__)

#else

#define ALLOC(dwBytes) GlobalAlloc(GPTR,(dwBytes))

#define REALLOC(hMem, dwBytes) GlobalReAlloc((hMem), (dwBytes), (GMEM_MOVEABLE|GMEM_ZEROINIT))

#define FREE(hMem)  GlobalFree((hMem))

#define CHECKFORLEAKS()

#define OOPS()
#endif



//*****************************************************************************
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

VOID
EnumerateHub (
    PCHAR                               HubName,
    PUSB_NODE_CONNECTION_INFORMATION    ConnectionInfo,
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc,
    PSTRING_DESCRIPTOR_NODE             StringDescs,
    PCHAR                               DeviceDesc
);

VOID
EnumerateHubPorts (
    HANDLE      hHubDevice,
    ULONG       NumPorts
);

PCHAR GetRootHubName (
    HANDLE HostController
);

PCHAR GetExternalHubName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
);

PCHAR GetHCDDriverKeyName (
    HANDLE  HCD
);

PCHAR GetDriverKeyName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
);

PUSB_DESCRIPTOR_REQUEST
GetConfigDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex
);

BOOL
AreThereStringDescriptors (
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
);

PSTRING_DESCRIPTOR_NODE
GetAllStringDescriptors (
    HANDLE                          hHubDevice,
    ULONG                           ConnectionIndex,
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
);

PSTRING_DESCRIPTOR_NODE
GetStringDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    USHORT  LanguageID
);

PSTRING_DESCRIPTOR_NODE
GetStringDescriptors (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    ULONG   NumLanguageIDs,
    USHORT  *LanguageIDs,
    PSTRING_DESCRIPTOR_NODE StringDescNodeTail
);

VOID DirectPort(
    HANDLE      hHubDevice,
    ULONG       Port
);
//
//	globals
//
ULONG TotalDevicesConnected;
UINT ConvertedId;
//char rawnbr[17];
UINT index;
char deviceName[500];
char tempName[500];

UINT Port, tempPort;

extern BOOL CheckKeyInfo(UINT vid, UINT pid);
extern UINT ProcessSerialNum(LPCSTR sernbr);
void AppendInfo(PUSBDEVICEINFO  pinfo)
{
	PSTRING_DESCRIPTOR_NODE Next;
	 if (!pinfo)
		 return;
    if (TotalDevicesConnected)	// already found one
		return;
    PUSB_NODE_CONNECTION_INFORMATION    ConnectInfo = pinfo->ConnectionInfo;
	if (!ConnectInfo)
		return;
	UINT vid = ConnectInfo->DeviceDescriptor.idVendor;
	UINT pid = ConnectInfo->DeviceDescriptor.idProduct;
	if (CheckKeyInfo(vid, pid)) // only look at our vendor and product
		return;
	ConvertedId = 0;
DPF("vid:%4X,pid:%4X",vid,pid);
    UINT ss = ConnectInfo->DeviceDescriptor.iSerialNumber;
DPF("ss:%d",ss);
//	tempPort = Port;
//	strcpy(tempName, deviceName);
//DPF("name:%s|",deviceName);
	Next = pinfo->StringDescs;
    for (;Next != NULL;)
        {
DPF("str index:%d",Next->DescriptorIndex);
DPF("lang id:%d",Next->LanguageID);
		if (Next->DescriptorIndex == ss)
			break;
		Next = Next->Next;
		}
	if (!Next)
		return;		// no serial number
	int i,c;
	char buf[33];
	if (Next->StringDescriptor[0].bDescriptorType!=USB_STRING_DESCRIPTOR_TYPE)
		return;
	c = Next->StringDescriptor[0].bLength;
	if (c < 2)
		return;
	c = (c - 2) / 2; // without blength and type and now bytes
	if (c > 32) c = 32;
	for (i = 0; i < c; i++)
		buf[i] = (char)Next->StringDescriptor[0].bString[i];
	buf[i] = 0;
	ConvertedId = ProcessSerialNum(buf);
	if (ConvertedId)
		{
		tempPort = Port;
		strcpy(tempName, deviceName);
    	TotalDevicesConnected++;
		}
}

UINT MyUSB_GetInfo(LPSTR buf,UINT size, UINT &port)
{
	DPF("getinfo:%s|",tempName);
	port = tempPort;
	UINT c = strlen(tempName);
	DPF("size:%d",c);
	if (c < size)
		strcpy(buf, tempName);
	else
		c = 0;
	return c;
}

UINT FindMyDevice(UINT & Id)
{
	char        HCName[16];
	int         HCNum;
    HANDLE      hHCDev;
    PCHAR       rootHubName;
    TotalDevicesConnected = 0;
//	pBuf = buf;

	for (HCNum = 0; HCNum < NUM_HCS_TO_CHECK; HCNum++)
	{
        wsprintf(HCName, "\\\\.\\HCD%d", HCNum);

        hHCDev = CreateFile(HCName,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

        // If the handle is valid, then we've successfully opened a Host
        // Controller.  Display some info about the Host Controller itself,
        // then enumerate the Root Hub attached to the Host Controller.
        //
        if (hHCDev != INVALID_HANDLE_VALUE)
        {
                rootHubName = GetRootHubName(hHCDev);
                if (rootHubName != NULL)
                {
                    EnumerateHub(
                                 rootHubName,
                                 NULL,      // ConnectionInfo
                                 NULL,      // ConfigDesc
                                 NULL,      // StringDescs
                                 "RootHub"  // DeviceDesc
                                );
                }

            CloseHandle(hHCDev);
        }
	}
	Id = ConvertedId;
	return TotalDevicesConnected;
}

UINT AccessMyDevice(UINT & Id, LPCSTR name, UINT port)
{
	HANDLE          hHubDevice;
//    BOOL            success;
//    ULONG           nBytes;
    PUSBDEVICEINFO  info;
    info        = NULL;
    hHubDevice  = INVALID_HANDLE_VALUE;
    TotalDevicesConnected = 0;
    // Allocate some space for a USBDEVICEINFO structure to hold the
    // hub info, hub name, and connection info pointers.  GPTR zero
    // initializes the structure for us.
    //
    info = (PUSBDEVICEINFO) ALLOC(sizeof(USBDEVICEINFO));

    if (info == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }


    // Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
    //
    info->HubInfo = (PUSB_NODE_INFORMATION)ALLOC(sizeof(USB_NODE_INFORMATION));

    if (info->HubInfo == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }
    
	hHubDevice = CreateFile(name,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

	if (hHubDevice == INVALID_HANDLE_VALUE)
	{
DPF("no temp open");
        OOPS();
        goto EnumerateHubError;
    }
	DirectPort(hHubDevice, port);

    CloseHandle(hHubDevice);
	Id = ConvertedId;
	return TotalDevicesConnected;

EnumerateHubError:
    //
    // Clean up any stuff that got allocated
    //

    if (hHubDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hHubDevice);
        hHubDevice = INVALID_HANDLE_VALUE;
    }

    if (info != NULL)
    {
        if (info->HubName != NULL)
        {
            FREE(info->HubName);
            info->HubName = NULL;
        }

        if (info->HubInfo != NULL)
        {
            FREE(info->HubInfo);
            info->HubInfo;
        }

        FREE(info);
        info = NULL;
    }
	return 0;
}




//*****************************************************************************
//
// EnumerateHub()
//
// hTreeParent - Handle of the TreeView item under which this hub should be
// added.
//
// HubName - Name of this hub.  This pointer is kept so the caller can neither
// free nor reuse this memory.
//
// ConnectionInfo - NULL if this is a root hub, else this is the connection
// info for an external hub.  This pointer is kept so the caller can neither
// free nor reuse this memory.
//
// ConfigDesc - NULL if this is a root hub, else this is the Configuration
// Descriptor for an external hub.  This pointer is kept so the caller can
// neither free nor reuse this memory.
//
//*****************************************************************************

VOID
EnumerateHub (
    PCHAR                               HubName,
    PUSB_NODE_CONNECTION_INFORMATION    ConnectionInfo,
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc,
    PSTRING_DESCRIPTOR_NODE             StringDescs,
    PCHAR                               DeviceDesc
)
{
	HANDLE          hHubDevice;
//    PCHAR           deviceName;
    BOOL            success;
    ULONG           nBytes;
    PUSBDEVICEINFO  info;
    info        = NULL;
    hHubDevice  = INVALID_HANDLE_VALUE;

    // Allocate some space for a USBDEVICEINFO structure to hold the
    // hub info, hub name, and connection info pointers.  GPTR zero
    // initializes the structure for us.
    //
    info = (PUSBDEVICEINFO) ALLOC(sizeof(USBDEVICEINFO));

    if (info == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }

    // Keep copies of the Hub Name, Connection Info, and Configuration
    // Descriptor pointers
    //
    info->HubName = HubName;

    info->ConnectionInfo = ConnectionInfo;

    info->ConfigDesc = ConfigDesc;

    info->StringDescs = StringDescs;


    // Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
    //
    info->HubInfo = (PUSB_NODE_INFORMATION)ALLOC(sizeof(USB_NODE_INFORMATION));

    if (info->HubInfo == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }
    
    // Allocate a temp buffer for the full hub device name.
    //
//    deviceName = (PCHAR)ALLOC(strlen(HubName) + sizeof("\\\\.\\"));

//    if (deviceName == NULL)
//    {
//        OOPS();
//        goto EnumerateHubError;
//    }

    // Create the full hub device name
    //
    strcpy(deviceName, "\\\\.\\");
    strcpy(deviceName + sizeof("\\\\.\\") - 1, info->HubName);

    // Try to hub the open device
    //
	hHubDevice = CreateFile(deviceName,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    // Done with temp buffer for full hub device name
    //
//    FREE(deviceName);

	if (hHubDevice == INVALID_HANDLE_VALUE)
	{
        OOPS();
        goto EnumerateHubError;
    }

    //
    // Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
    // This will tell us the number of downstream ports to enumerate, among
    // other things.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_NODE_INFORMATION,
                              info->HubInfo,
                              sizeof(USB_NODE_INFORMATION),
                              info->HubInfo,
                              sizeof(USB_NODE_INFORMATION),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto EnumerateHubError;
    }
    
//	AppendInfo(info);

    // Now recursively enumrate the ports of this hub.
    // 
    EnumerateHubPorts(
        hHubDevice,
        info->HubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts
        );


    CloseHandle(hHubDevice);
    return;

EnumerateHubError:
    //
    // Clean up any stuff that got allocated
    //

    if (hHubDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hHubDevice);
        hHubDevice = INVALID_HANDLE_VALUE;
    }

    if (info != NULL)
    {
        if (info->HubName != NULL)
        {
            FREE(info->HubName);
            info->HubName = NULL;
        }

        if (info->HubInfo != NULL)
        {
            FREE(info->HubInfo);
            info->HubInfo;
        }

        FREE(info);
        info = NULL;
    }

    if (ConnectionInfo)
    {
        FREE(ConnectionInfo);
    }

    if (ConfigDesc)
    {
        FREE(ConfigDesc);
    }

    if (StringDescs != NULL)
    {
        PSTRING_DESCRIPTOR_NODE Next;

        do {

            Next = StringDescs->Next;
            FREE(StringDescs);
            StringDescs = Next;

        } while (StringDescs != NULL);
    }
}

VOID DirectPort(
    HANDLE      hHubDevice,
    ULONG       index
)
{
    BOOL        success;
DPF("direct:%d",index);
    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
    PUSB_DESCRIPTOR_REQUEST             configDesc;
    PSTRING_DESCRIPTOR_NODE             stringDescs;
    PUSBDEVICEINFO                      info;

    PCHAR driverKeyName;
    PCHAR deviceDesc;
        ULONG nBytes;

        // Allocate space to hold the connection info for this port.
        // For now, allocate it big enough to hold info for 16 pipes.
        // Should probably size this dynamically at some point.
        //
        nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
                 sizeof(USB_PIPE_INFO) * 16;

        connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)ALLOC(nBytes);

        if (connectionInfo == NULL)
        {
            OOPS();
           return;// break;
        }

        //
        // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION structure
        // for this port.  This will tell us if a device is attached to this
        // port, among other things.
        //
        connectionInfo->ConnectionIndex = index;

        success = DeviceIoControl(hHubDevice,
                                  IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                                  connectionInfo,
                                  nBytes,
                                  connectionInfo,
                                  nBytes,
                                  &nBytes,
                                  NULL);

        if (!success)
        {
            FREE(connectionInfo);
            return;//continue;
        }

        // Update the count of connected devices
        //
//        if (connectionInfo->ConnectionStatus == DeviceConnected)
//        {
//            TotalDevicesConnected++;
//        }

        // If there is a device connected, get the Device Description
        //
        deviceDesc = NULL;
        if (connectionInfo->ConnectionStatus != NoDeviceConnected)
        {
            driverKeyName = GetDriverKeyName(hHubDevice,
                                             index);

            if (driverKeyName)
            {
                deviceDesc = 0;//DriverNameToDeviceDesc(driverKeyName);
//DPF("name:%|",driverKeyName);
                FREE(driverKeyName);
            }
        }

        // If there is a device connected to the port, try to retrieve the
        // Configuration Descriptor from the device.
        //
		if(connectionInfo->ConnectionStatus == DeviceConnected)
        	{
            configDesc = GetConfigDescriptor(hHubDevice,
                                             index,
                                             0);
        	}
        else
        	{
            configDesc = NULL;
        	}

        if (configDesc != NULL &&
            AreThereStringDescriptors(&connectionInfo->DeviceDescriptor,
                                      (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1)))
        	{
            stringDescs = GetAllStringDescriptors(
                              hHubDevice,
                              index,
                              &connectionInfo->DeviceDescriptor,
                              (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1));
        	}
        else
        	{
            stringDescs = NULL;
        	}

        // If the device connected to the port is an external hub, get the
        // name of the external hub and recursively enumerate it.
        //
        if (connectionInfo->DeviceIsHub)
        {
            PCHAR extHubName;

            extHubName = GetExternalHubName(hHubDevice,
                                            index);

            if (extHubName != NULL)
            {
                EnumerateHub(
                             extHubName,
                             connectionInfo,
                             configDesc,
                             stringDescs,
                             deviceDesc);

                // On to the next port
                //
               return;// continue;
            }
        }

        // Allocate some space for a USBDEVICEINFO structure to hold the
        // hub info, hub name, and connection info pointers.  GPTR zero
        // initializes the structure for us.
        //
        info = (PUSBDEVICEINFO) ALLOC(sizeof(USBDEVICEINFO));

        if (info == NULL)
        {
            OOPS();
            if (configDesc != NULL)
            {
                FREE(configDesc);
            }
            FREE(connectionInfo);
            return;//break;
        }

        info->ConnectionInfo = connectionInfo;

        info->ConfigDesc = configDesc;

        info->StringDescs = stringDescs;
        
	AppendInfo(info);
    
}



//*****************************************************************************
//
// EnumerateHubPorts()
//
// hTreeParent - Handle of the TreeView item under which the hub port should
// be added.
//
// hHubDevice - Handle of the hub device to enumerate.
//
// NumPorts - Number of ports on the hub.
//
//*****************************************************************************

VOID
EnumerateHubPorts (
    HANDLE      hHubDevice,
    ULONG       NumPorts
)
{
    ULONG       index;
    BOOL        success;
DPF("enum ports,cnt:%d",NumPorts);
    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
    PUSB_DESCRIPTOR_REQUEST             configDesc;
    PSTRING_DESCRIPTOR_NODE             stringDescs;
    PUSBDEVICEINFO                      info;

    PCHAR driverKeyName;
    PCHAR deviceDesc;
    // Loop over all ports of the hub.
    //
    // Port indices are 1 based, not 0 based.
    //
    for (index=1; index <= NumPorts; index++)
    {
		Port = index;
        ULONG nBytes;

        // Allocate space to hold the connection info for this port.
        // For now, allocate it big enough to hold info for 16 pipes.
        // Should probably size this dynamically at some point.
        //
        nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
                 sizeof(USB_PIPE_INFO) * 16;

        connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)ALLOC(nBytes);

        if (connectionInfo == NULL)
        {
            OOPS();
            break;
        }

        //
        // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION structure
        // for this port.  This will tell us if a device is attached to this
        // port, among other things.
        //
        connectionInfo->ConnectionIndex = index;

        success = DeviceIoControl(hHubDevice,
                                  IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                                  connectionInfo,
                                  nBytes,
                                  connectionInfo,
                                  nBytes,
                                  &nBytes,
                                  NULL);

        if (!success)
        {
            FREE(connectionInfo);
            continue;
        }

        // Update the count of connected devices
        //
//        if (connectionInfo->ConnectionStatus == DeviceConnected)
//        {
//            TotalDevicesConnected++;
//        }

        // If there is a device connected, get the Device Description
        //
        deviceDesc = NULL;
        if (connectionInfo->ConnectionStatus != NoDeviceConnected)
        {
            driverKeyName = GetDriverKeyName(hHubDevice,
                                             index);

            if (driverKeyName)
            {
                deviceDesc = 0;//DriverNameToDeviceDesc(driverKeyName);
//DPF("name:%|",driverKeyName);
                FREE(driverKeyName);
            }
        }

        // If there is a device connected to the port, try to retrieve the
        // Configuration Descriptor from the device.
        //
		if(connectionInfo->ConnectionStatus == DeviceConnected)
        	{
            configDesc = GetConfigDescriptor(hHubDevice,
                                             index,
                                             0);
        	}
        else
        	{
            configDesc = NULL;
        	}

        if (configDesc != NULL &&
            AreThereStringDescriptors(&connectionInfo->DeviceDescriptor,
                                      (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1)))
        	{
            stringDescs = GetAllStringDescriptors(
                              hHubDevice,
                              index,
                              &connectionInfo->DeviceDescriptor,
                              (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1));
        	}
        else
        	{
            stringDescs = NULL;
        	}

        // If the device connected to the port is an external hub, get the
        // name of the external hub and recursively enumerate it.
        //
        if (connectionInfo->DeviceIsHub)
        {
            PCHAR extHubName;

            extHubName = GetExternalHubName(hHubDevice,
                                            index);

            if (extHubName != NULL)
            {
                EnumerateHub(
                             extHubName,
                             connectionInfo,
                             configDesc,
                             stringDescs,
                             deviceDesc);

                // On to the next port
                //
                continue;
            }
        }

        // Allocate some space for a USBDEVICEINFO structure to hold the
        // hub info, hub name, and connection info pointers.  GPTR zero
        // initializes the structure for us.
        //
        info = (PUSBDEVICEINFO) ALLOC(sizeof(USBDEVICEINFO));

        if (info == NULL)
        {
            OOPS();
            if (configDesc != NULL)
            {
                FREE(configDesc);
            }
            FREE(connectionInfo);
            break;
        }

        info->ConnectionInfo = connectionInfo;

        info->ConfigDesc = configDesc;

        info->StringDescs = stringDescs;
        
	AppendInfo(info);
    }
}


//*****************************************************************************
//
// WideStrToMultiStr()
//
//*****************************************************************************

PCHAR WideStrToMultiStr (PWCHAR WideStr)
{
    ULONG nBytes;
    PCHAR MultiStr;

    // Get the length of the converted string
    //
    nBytes = WideCharToMultiByte(
                 CP_ACP,
                 0,
                 WideStr,
                 -1,
                 NULL,
                 0,
                 NULL,
                 NULL);

    if (nBytes == 0)
    {
        return NULL;
    }
    
    // Allocate space to hold the converted string
    //
    MultiStr = (char *)ALLOC(nBytes);

    if (MultiStr == NULL)
    {
        return NULL;
    }

    // Convert the string
    //
    nBytes = WideCharToMultiByte(
                 CP_ACP,
                 0,
                 WideStr,
                 -1,
                 MultiStr,
                 nBytes,
                 NULL,
                 NULL);

    if (nBytes == 0)
    {
        FREE(MultiStr);
        return NULL;
    }

    return MultiStr;
}


//*****************************************************************************
//
// GetRootHubName()
//
//*****************************************************************************

PCHAR GetRootHubName (
    HANDLE HostController
)
{
    BOOL                success;
    ULONG               nBytes;
    USB_ROOT_HUB_NAME   rootHubName;
    PUSB_ROOT_HUB_NAME  rootHubNameW;
    PCHAR               rootHubNameA;

    rootHubNameW = NULL;
    rootHubNameA = NULL;

    // Get the length of the name of the Root Hub attached to the
    // Host Controller
    //
    success = DeviceIoControl(HostController,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              0,
                              0,
                              &rootHubName,
                              sizeof(rootHubName),
                              &nBytes,
                              NULL);
    
    if (!success)
    {
        OOPS();
        goto GetRootHubNameError;
    }

    // Allocate space to hold the Root Hub name
    //
    nBytes = rootHubName.ActualLength;

    rootHubNameW = (USB_ROOT_HUB_NAME *)ALLOC(nBytes);

    if (rootHubNameW == NULL)
    {
        OOPS();
        goto GetRootHubNameError;
    }

    // Get the name of the Root Hub attached to the Host Controller
    //
    success = DeviceIoControl(HostController,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              NULL,
                              0,
                              rootHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);
    
    if (!success)
    {
        OOPS();
        goto GetRootHubNameError;
    }

    // Convert the Root Hub name
    //
    rootHubNameA = WideStrToMultiStr(rootHubNameW->RootHubName);

    // All done, free the uncoverted Root Hub name and return the
    // converted Root Hub name
    //
    FREE(rootHubNameW);

    return rootHubNameA;

    
GetRootHubNameError:
    // There was an error, free anything that was allocated
    //
    if (rootHubNameW != NULL)
    {
        FREE(rootHubNameW);
        rootHubNameW = NULL;
    }
    
    return NULL;
}


//*****************************************************************************
//
// GetExternalHubName()
//
//*****************************************************************************

PCHAR GetExternalHubName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
)
{
    BOOL                        success;
    ULONG                       nBytes;
    USB_NODE_CONNECTION_NAME    extHubName;
    PUSB_NODE_CONNECTION_NAME   extHubNameW;
    PCHAR                       extHubNameA;

    extHubNameW = NULL;
    extHubNameA = NULL;

    // Get the length of the name of the external hub attached to the
    // specified port.
    //
    extHubName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              &extHubName,
                              sizeof(extHubName),
                              &extHubName,
                              sizeof(extHubName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    // Allocate space to hold the external hub name
    //
    nBytes = extHubName.ActualLength;

    if (nBytes <= sizeof(extHubName))
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    extHubNameW = (USB_NODE_CONNECTION_NAME *)ALLOC(nBytes);

    if (extHubNameW == NULL)
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    // Get the name of the external hub attached to the specified port
    //
    extHubNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              extHubNameW,
                              nBytes,
                              extHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);
    
    if (!success)
    {
        OOPS();
        goto GetExternalHubNameError;
    }
    
    // Convert the External Hub name
    //
    extHubNameA = WideStrToMultiStr(extHubNameW->NodeName);

    // All done, free the uncoverted external hub name and return the
    // converted external hub name
    //
    FREE(extHubNameW);

    return extHubNameA;

    
GetExternalHubNameError:
    // There was an error, free anything that was allocated
    //
    if (extHubNameW != NULL)
    {
        FREE(extHubNameW);
        extHubNameW = NULL;
    }
    
    return NULL;
}


//*****************************************************************************
//
// GetDriverKeyName()
//
//*****************************************************************************

PCHAR GetDriverKeyName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
)
{
    BOOL                                success;
    ULONG                               nBytes;
    USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;
    PCHAR                               driverKeyNameA;

    driverKeyNameW = NULL;
    driverKeyNameA = NULL;

    // Get the length of the name of the driver key of the device attached to
    // the specified port.
    //
    driverKeyName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    // Allocate space to hold the driver key name
    //
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    driverKeyNameW = (USB_NODE_CONNECTION_DRIVERKEY_NAME *)ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    // Get the name of the driver key of the device attached to
    // the specified port.
    //
    driverKeyNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);
    
    if (!success)
    {
        OOPS();
        goto GetDriverKeyNameError;
    }
    
    // Convert the driver key name
    //
    driverKeyNameA = WideStrToMultiStr(driverKeyNameW->DriverKeyName);

    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    //
    FREE(driverKeyNameW);

    return driverKeyNameA;

    
GetDriverKeyNameError:
    // There was an error, free anything that was allocated
    //
    if (driverKeyNameW != NULL)
    {
        FREE(driverKeyNameW);
        driverKeyNameW = NULL;
    }
    
    return NULL;
}


//*****************************************************************************
//
// GetHCDDriverKeyName()
//
//*****************************************************************************

PCHAR GetHCDDriverKeyName (
    HANDLE  HCD
)
{
    BOOL                    success;
    ULONG                   nBytes;
    USB_HCD_DRIVERKEY_NAME  driverKeyName;
    PUSB_HCD_DRIVERKEY_NAME driverKeyNameW;
    PCHAR                   driverKeyNameA;

    driverKeyNameW = NULL;
    driverKeyNameA = NULL;

    // Get the length of the name of the driver key of the HCD
    //
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    // Allocate space to hold the driver key name
    //
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameW = (USB_HCD_DRIVERKEY_NAME*)ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    // Get the name of the driver key of the device attached to
    // the specified port.
    //
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);
    
    if (!success)
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }
    
    // Convert the driver key name
    //
    driverKeyNameA = WideStrToMultiStr(driverKeyNameW->DriverKeyName);

    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    //
    FREE(driverKeyNameW);

    return driverKeyNameA;

    
GetHCDDriverKeyNameError:
    // There was an error, free anything that was allocated
    //
    if (driverKeyNameW != NULL)
    {
        FREE(driverKeyNameW);
        driverKeyNameW = NULL;
    }
    
    return NULL;
}


//*****************************************************************************
//
// GetConfigDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// Configuration Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the Configuration Descriptor will be requested.
//
// DescriptorIndex - Configuration Descriptor index, zero based.
//
//*****************************************************************************

PUSB_DESCRIPTOR_REQUEST
GetConfigDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex
)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;
DPF("get config desc,%d",ConnectionIndex);

    UCHAR   configDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
                             sizeof(USB_CONFIGURATION_DESCRIPTOR)];

    PUSB_DESCRIPTOR_REQUEST         configDescReq;
    PUSB_CONFIGURATION_DESCRIPTOR   configDesc;


    // Request the Configuration Descriptor the first time using our
    // local buffer, which is just big enough for the Cofiguration
    // Descriptor itself.
    //
    nBytes = sizeof(configDescReqBuf);

    configDescReq = (PUSB_DESCRIPTOR_REQUEST)configDescReqBuf;
    configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

    // Zero fill the entire request structure
    //
    memset(configDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    configDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);
    
    if (!success)
    {
        OOPS();
        return NULL;
    }

    if (nBytes != nBytesReturned)
    {
        OOPS();
        return NULL;
    }

    if (configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        OOPS();
        return NULL;
    }

    // Now request the entire Configuration Descriptor using a dynamically
    // allocated buffer which is sized big enough to hold the entire descriptor
    //
    nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + configDesc->wTotalLength;

    configDescReq = (PUSB_DESCRIPTOR_REQUEST)ALLOC(nBytes);

    if (configDescReq == NULL)
    {
        OOPS();
        return NULL;
    }

    configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

    // Indicate the port from which the descriptor will be requested
    //
    configDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);
    
    if (!success)
    {
        OOPS();
        FREE(configDescReq);
        return NULL;
    }

    if (nBytes != nBytesReturned)
    {
        OOPS();
        FREE(configDescReq);
        return NULL;
    }

    if (configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
    {
        OOPS();
        FREE(configDescReq);
        return NULL;
    }
    return configDescReq;
}


//*****************************************************************************
//
// AreThereStringDescriptors()
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// checked.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be checked.
//
//*****************************************************************************

BOOL
AreThereStringDescriptors (
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;

    //
    // Check Device Descriptor strings
    //

    if (DeviceDesc->iManufacturer ||
        DeviceDesc->iProduct      ||
        DeviceDesc->iSerialNumber
       )
    {
        return TRUE;
    }


    //
    // Check the Configuration and Interface Descriptor strings
    //

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;
    
    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

	PUCHAR pTemp = (PUCHAR)commonDesc;
	for (;;)
		{
		commonDesc = (PUSB_COMMON_DESCRIPTOR)pTemp;
		if ((pTemp >= descEnd) || ( pTemp + commonDesc->bLength > descEnd))
			break;
        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    return TRUE;
                }
				pTemp += commonDesc->bLength;
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    return TRUE;
                }
                pTemp += commonDesc->bLength;
                continue;

            default:
                pTemp += commonDesc->bLength;
                continue;
        }
        break;
    }

    return FALSE;
}


//*****************************************************************************
//
// GetAllStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptors will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptors will be requested.
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// requested.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetAllStringDescriptors (
    HANDLE                          hHubDevice,
    ULONG                           ConnectionIndex,
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PSTRING_DESCRIPTOR_NODE supportedLanguagesString;
    PSTRING_DESCRIPTOR_NODE stringDescNodeTail;
    ULONG                   numLanguageIDs;
    USHORT                  *languageIDs;

//    PUCHAR                  descEnd;
//    PUSB_COMMON_DESCRIPTOR  commonDesc;

    //
    // Get the array of supported Language IDs, which is returned
    // in String Descriptor 0
    //
    supportedLanguagesString = GetStringDescriptor(hHubDevice,
                                                   ConnectionIndex,
                                                   0,
                                                   0);

    if (supportedLanguagesString == NULL)
    {
        return NULL;
    }

    numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;

    languageIDs = (USHORT *)&supportedLanguagesString->StringDescriptor->bString[0];

    stringDescNodeTail = supportedLanguagesString;

    //
    // Get the Device Descriptor strings
    //
/*
    if (DeviceDesc->iManufacturer)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iManufacturer,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
    }

    if (DeviceDesc->iProduct)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iProduct,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
    }
*/


    if (DeviceDesc->iSerialNumber)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iSerialNumber,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
    }

/*
    //
    // Get the Configuration and Interface Descriptor strings
    //

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;
    

	PUCHAR pTemp = (PUCHAR)ConfigDesc;
	for (;;)
		{
	    commonDesc = (PUSB_COMMON_DESCRIPTOR)pTemp;
    	if (pTemp + sizeof(USB_COMMON_DESCRIPTOR) >= descEnd ||
           	pTemp + commonDesc->bLength > descEnd)
			break;
        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    stringDescNodeTail = GetStringDescriptors(
                                             hHubDevice,
                                             ConnectionIndex,
                                             ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
                                             numLanguageIDs,
                                             languageIDs,
                                             stringDescNodeTail);
                }
                pTemp += commonDesc->bLength;
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    stringDescNodeTail = GetStringDescriptors(
                                             hHubDevice,
                                             ConnectionIndex,
                                             ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface,
                                             numLanguageIDs,
                                             languageIDs,
                                             stringDescNodeTail);
                }
                pTemp += commonDesc->bLength;
                continue;

            default:
                pTemp += commonDesc->bLength;
                continue;
        }
        break;
    }
*/
    return supportedLanguagesString;
}


//*****************************************************************************
//
// GetStringDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// LanguageID - Language in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetStringDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    USHORT  LanguageID
)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
                             MAXIMUM_USB_STRING_LENGTH];

    PUSB_DESCRIPTOR_REQUEST stringDescReq;
    PUSB_STRING_DESCRIPTOR  stringDesc;
    PSTRING_DESCRIPTOR_NODE stringDescNode;

    nBytes = sizeof(stringDescReqBuf);

    stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
    stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

    // Zero fill the entire request structure
    //
    memset(stringDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    stringDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    stringDescReq->SetupPacket.wIndex = LanguageID;

    stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              stringDescReq,
                              nBytes,
                              stringDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    //
    // Do some sanity checks on the return from the get descriptor request.
    //

    if (!success)
    {
        OOPS();
        return NULL;
    }

    if (nBytesReturned < 2)
    {
        OOPS();
        return NULL;
    }

    if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
    {
        OOPS();
        return NULL;
    }

    if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
    {
        OOPS();
        return NULL;
    }

    if (stringDesc->bLength % 2 != 0)
    {
        OOPS();
        return NULL;
    }

    //
    // Looks good, allocate some (zero filled) space for the string descriptor
    // node and copy the string descriptor to it.
    //

    stringDescNode = (PSTRING_DESCRIPTOR_NODE)ALLOC(sizeof(STRING_DESCRIPTOR_NODE) +
                                                    stringDesc->bLength);

    if (stringDescNode == NULL)
    {
        OOPS();
        return NULL;
    }
   
    stringDescNode->DescriptorIndex = DescriptorIndex;
    stringDescNode->LanguageID = LanguageID;

    memcpy(stringDescNode->StringDescriptor,
           stringDesc,
           stringDesc->bLength);

    return stringDescNode;
}


//*****************************************************************************
//
// GetStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// NumLanguageIDs -  Number of languages in which the string should be
// requested.
//
// LanguageIDs - Languages in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetStringDescriptors (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    ULONG   NumLanguageIDs,
    USHORT  *LanguageIDs,
    PSTRING_DESCRIPTOR_NODE StringDescNodeTail
)
{
    ULONG i;

    for (i=0; i<NumLanguageIDs; i++)
    {
        StringDescNodeTail->Next = GetStringDescriptor(hHubDevice,
                                                       ConnectionIndex,
                                                       DescriptorIndex,
                                                       *LanguageIDs);

        if (StringDescNodeTail->Next)
        {
            StringDescNodeTail = StringDescNodeTail->Next;
        }
        
        LanguageIDs++;
    }

    return StringDescNodeTail;
}
#endif
