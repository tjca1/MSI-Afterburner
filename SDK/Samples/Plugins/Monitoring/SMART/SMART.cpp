// SMART.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
 
#include "SMART.h"
#include "MAHMSharedMemory.h"
#include "MSIAfterburnerMonitoringSourceDesc.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE SMARTDLL = { NULL, NULL };
/////////////////////////////////////////////////////////////////////////////
#define MAX_DRIVE											16
/////////////////////////////////////////////////////////////////////////////
HINSTANCE					g_hModule						= 0;
DWORD						g_dwPhysicalDrives[MAX_DRIVE]	= { 0 };
DWORD						g_dwNumberOfDrives				= 0;

FLOAT						g_fltTemperature[MAX_DRIVE]		= { FLT_MAX };
DWORD						g_dwTickCount[MAX_DRIVE]		= { 0 };
DWORD						g_dwMinPollingInterval			= 0;
/////////////////////////////////////////////////////////////////////////////
//
// S.M.A.R.T. related constants and structures required to read attributes
//
/////////////////////////////////////////////////////////////////////////////
#define READ_ATTRIBUTE_BUFFER_SIZE		512
/////////////////////////////////////////////////////////////////////////////
#define DFP_RECEIVE_DRIVE_DATA			0x0007c088
/////////////////////////////////////////////////////////////////////////////
#define IDE_EXECUTE_SMART_FUNCTION		0xB0
/////////////////////////////////////////////////////////////////////////////
#define SMART_CYL_LOW					0x4F
#define SMART_CYL_HI					0xC2
/////////////////////////////////////////////////////////////////////////////
#define	SMART_READ_ATTRIBUTE_VALUES		0xD0
/////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
/////////////////////////////////////////////////////////////////////////////
typedef struct IDEREGS
{
   BYTE					FeaturesReg;     
   BYTE					SectorCountReg;  
   BYTE					SectorNumberReg; 
   BYTE					CylLowReg;       
   BYTE					CylHighReg;      
   BYTE					DriveHeadReg;    
   BYTE					CommandReg;      
   BYTE					Reserved;        
} IDEREGS, *LPIDEREGS;
/////////////////////////////////////////////////////////////////////////////
typedef struct SENDCMDINPARAMS
{
   DWORD				BufferSize;   
   IDEREGS				DriveRegs;   
   BYTE					DriveNumber;                       
   BYTE					Reserved0[3];  
   DWORD				Reserved1[4]; 
} SENDCMDINPARAMS, *LPSENDCMDINPARAMS;
/////////////////////////////////////////////////////////////////////////////
typedef struct DRIVERSTATUS
{
   BYTE					DriverError;
   BYTE					IDEStatus;                
   BYTE					Reserved0[2]; 
   DWORD				Reserved1[2];
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;
/////////////////////////////////////////////////////////////////////////////
typedef struct SENDCMDOUTPARAMS
{
   DWORD				BufferSize; 
   DRIVERSTATUS			DriverStatus;
} SENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
/////////////////////////////////////////////////////////////////////////////
typedef struct  DRIVEATTRIBUTEHDR
{
    WORD				Revision;   
} DRIVEATTRIBUTEHDR, *LPDRIVEATTRIBUTEHDR;
/////////////////////////////////////////////////////////////////////////////
typedef struct  DRIVEATTRIBUTE
{
    BYTE				AttrID;
    WORD				StatusFlags;
    BYTE				AttrValue;  
    BYTE				WorstValue; 
    BYTE				RawValue[6];
    BYTE				Reserved;   
} DRIVEATTRIBUTE, *LPDRIVEATTRIBUTE;
/////////////////////////////////////////////////////////////////////////////
typedef struct READATTRIBUTEBUFFER
{
	DRIVEATTRIBUTEHDR	Hdr;
	DRIVEATTRIBUTE		Attr[30];
	BYTE				Reserved[150];
} READATTRIBUTEBUFFER, *LPREADATTRIBUTEBUFFER;
/////////////////////////////////////////////////////////////////////////////
typedef struct SENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES
{
	SENDCMDOUTPARAMS	SendCmdOutParams;
	READATTRIBUTEBUFFER	ReadAttributeBuffer;
} SENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES, *LPSENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES;
/////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to read hard drive temperature by physical 
// drive index 
/////////////////////////////////////////////////////////////////////////////
float GetTemperature(BYTE drive)
{
	//format drive name

	char szDrive[MAX_PATH];
	sprintf_s(szDrive, "\\\\.\\PhysicalDrive%d", drive);

	//try to get drive handle and return error on failure

	HANDLE hDrive = CreateFile (szDrive, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive == INVALID_HANDLE_VALUE)
		return FLT_MAX;

	//declare and init input IOCTL parameters

	SENDCMDINPARAMS inp;
	memset(&inp, 0, sizeof(inp));

	inp.BufferSize					= READ_ATTRIBUTE_BUFFER_SIZE;
	inp.DriveRegs.FeaturesReg		= SMART_READ_ATTRIBUTE_VALUES;
	inp.DriveRegs.SectorCountReg	= 1;
	inp.DriveRegs.SectorNumberReg	= 1;
	inp.DriveRegs.CylLowReg			= SMART_CYL_LOW;
	inp.DriveRegs.CylHighReg		= SMART_CYL_HI;
	inp.DriveRegs.DriveHeadReg		= 0xA0 | ((drive & 1)<<4);
	inp.DriveRegs.CommandReg		= IDE_EXECUTE_SMART_FUNCTION;
	inp.DriveNumber					= drive;

	//declare and init IOCTL output parameters

	SENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES out;
	memset(&out, 0, sizeof(out));

	//try to read S.M.A.R.T. attributes via DFP_RECEIVE_DRIVE_DATA IOCL and return error on failure

	DWORD dwBytesReturned;
	if (!DeviceIoControl (hDrive, DFP_RECEIVE_DRIVE_DATA, &inp, sizeof(inp), &out, sizeof(out), &dwBytesReturned, NULL))
	{
		CloseHandle(hDrive);
		return FLT_MAX;
	}

	CloseHandle(hDrive);

	//loop through attributes array and search for temeprature attribute

	for (DWORD dwAttr=0; dwAttr<30; dwAttr++)
	{
		if ((out.ReadAttributeBuffer.Attr[dwAttr].AttrID == 0xC2) ||
			(out.ReadAttributeBuffer.Attr[dwAttr].AttrID == 0xE7) ||
			(out.ReadAttributeBuffer.Attr[dwAttr].AttrID == 0xBE))
			//process temperature attribute
		{
			WORD temp = *((LPWORD)out.ReadAttributeBuffer.Attr[dwAttr].RawValue);
				//get raw temperature (can be either in 1°C or in 0.1°C units depending on hard drive manufacturer)
			
			if (temp > 200)
				//convert 0.1°C based temperature format to 1°C based
				temp = temp / 10;

			return temp;
		}
	}

	return FLT_MAX;
}
//////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(SMARTDLL, hInstance))
			return 0;

		new CDynLinkLibrary(SMARTDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(SMARTDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get a number of
// data sources in this plugin
//////////////////////////////////////////////////////////////////////
SMART_API DWORD GetSourcesNum()
{
	return MAX_DRIVE;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get descriptor
// for the specified data source
//////////////////////////////////////////////////////////////////////
SMART_API BOOL GetSourceDesc(DWORD dwIndex, LPMONITORING_SOURCE_DESC pDesc)
{
	//init global variables, which will be used for HDD temperature monitoring

	if (!g_dwNumberOfDrives)
		//init physical drives array if it is not initialized yet
	{
		for (BYTE drive=0; drive<MAX_DRIVE; drive++)
		{
			if (GetTemperature(drive) != FLT_MAX)
				g_dwPhysicalDrives[g_dwNumberOfDrives++] = drive;
		}

		//get fully qualified path to .cfg
		char szCfgPath[MAX_PATH];
		GetModuleFileName(g_hModule, szCfgPath, MAX_PATH);
		
		PathRenameExtension(szCfgPath, ".cfg");

		//load plugin settings
		g_dwMinPollingInterval = GetPrivateProfileInt("Settings", "MinPollingInterval", 0, szCfgPath);
	}

	if (dwIndex < g_dwNumberOfDrives)
	{
		DWORD dwPhysicalDrive = g_dwPhysicalDrives[dwIndex];

		sprintf_s(pDesc->szName		, "HDD%d temperature"	, dwPhysicalDrive + 1);
		sprintf_s(pDesc->szGroup	, "HDD%d"				, dwPhysicalDrive + 1);

		strcpy_s(pDesc->szNameTemplate	, sizeof(pDesc->szNameTemplate), "HDD%d temperature");
		strcpy_s(pDesc->szGroupTemplate	, sizeof(pDesc->szNameTemplate), "HDD%d");

		strcpy_s(pDesc->szUnits			, sizeof(pDesc->szUnits), "°C");

		pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_HDD;
		pDesc->dwInstance	= dwPhysicalDrive;

		pDesc->fltMaxLimit	= 100.0f;
		pDesc->fltMinLimit	= 0.0f;

		return TRUE;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to poll data sources
/////////////////////////////////////////////////////////////////////////////
SMART_API FLOAT GetSourceData(DWORD dwIndex)
{
	DWORD dwTickCount = GetTickCount();
		//get standard timer tick count

	if (dwTickCount - g_dwTickCount[dwIndex] >= g_dwMinPollingInterval)
		//update temperature using specified minimum polling interval
	{
		FLOAT fltTemperature = GetTemperature((BYTE)g_dwPhysicalDrives[dwIndex]);

		if (fltTemperature != FLT_MAX)
			g_fltTemperature[dwIndex] = fltTemperature;
			//save temeprature, don't forget to filter possible misreadings (e.g. when drive is sleeping)

		g_dwTickCount[dwIndex] = dwTickCount;
			//save last polling timestamp
	}

	return g_fltTemperature[dwIndex];
}
/////////////////////////////////////////////////////////////////////////////
