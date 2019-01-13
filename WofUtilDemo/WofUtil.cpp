/*
 * PROJECT:   WofUtilDemo
 * FILE:      WofUtil.cpp
 * PURPOSE:   Reverse Engineering of WofUtil.dll
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#include <Windows.h>
#include <winioctl.h>

#include "wofapi_sdk.h"

/*
WofComputeSystemPolicy
WofConfigureSystemPolicy
WofDecompressRateWorker
WofEnumEntries
WofEvaluateDecompressionRate
WofFileEnumFiles
WofGetDWordFromRegistry
WofGetDecompressionRate
WofGetDriverVersion - Reversed
WofInitializeCompressedBuffer
WofInitializePolicyThresholds
WofIsExternalFile
WofMeasureDecompressionRate
WofQueryNoSeekLatency
WofRequestPerfBoost
WofSetDWordInRegistry
WofSetDWordInRegistryTest
WofSetFileDataLocation
WofShouldCompressBinaries
WofShouldCompressBinariesEx
WofStoreEvalData
WofWimAddEntry
WofWimEnumFiles
WofWimRemoveEntry - Reversed
WofWimSuspendEntry - Reversed
WofWimUpdateEntry
WofpBuildNameFromComponents
WofpDeviceIoControl - Reversed
WofpEnumFiles
WofpFilterAttach
WofpFindFirstFileName
WofpFindNextFileName
WofpFindParent
WofpIsExternalFileByHandle
WofpNormalizeFilePath
WofpOpenByFileId
WofpOpenVolumeWithFlagsAndAttributes - Reversed
WofpQueryNameInformation
*/

HRESULT WINAPI WofGetDriverVersion(
    __in HANDLE FileOrVolumeHandle,
    __in ULONG Provider,
    __out PULONG WofVersion)
{
    WOF_EXTERNAL_INFO InBuffer;
    WOF_VERSION_INFO OutBuffer;
    DWORD BytesReturned;

    InBuffer.Provider = Provider;
    InBuffer.Version = WOF_CURRENT_VERSION;

    OutBuffer.WofVersion = 0;
    
    if (DeviceIoControl(
        FileOrVolumeHandle,
        FSCTL_GET_WOF_VERSION,
        &InBuffer,
        sizeof(WOF_EXTERNAL_INFO),
        &OutBuffer,
        sizeof(WOF_VERSION_INFO),
        &BytesReturned,
        nullptr))
    {
        if (BytesReturned >= sizeof(WOF_VERSION_INFO))
        {
            *WofVersion = OutBuffer.WofVersion;
            return S_OK;
        }
        else
        {
            return __HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        }
    }
    else
    {
        return  __HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

HRESULT WINAPI WofpDeviceIoControl(
    _In_ HANDLE hDevice,
    _In_ DWORD dwIoControlCode,
    _In_reads_bytes_opt_(nInBufferSize) LPVOID lpInBuffer,
    _In_ DWORD nInBufferSize,
    _Out_writes_bytes_to_opt_(nOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
    _In_ DWORD nOutBufferSize,
    _Out_opt_ LPDWORD lpBytesReturned)
{
    if (!DeviceIoControl(
        hDevice,
        dwIoControlCode,
        lpInBuffer,
        nInBufferSize,
        lpOutBuffer,
        nOutBufferSize,
        lpBytesReturned,
        nullptr))
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HANDLE WINAPI WofpOpenVolumeWithFlagsAndAttributes(
    _In_ LPCWSTR lpszFileName,
    _In_ BOOL NeedToWrite)
{
    HANDLE hVolume = INVALID_HANDLE_VALUE;

    LPWSTR VolumeMountPoint = reinterpret_cast<LPWSTR>(HeapAlloc(
        GetProcessHeap(),
        0,
        MAX_PATH * sizeof(wchar_t)));
    if (VolumeMountPoint)
    {
        if (GetVolumePathNameW(
            lpszFileName,
            VolumeMountPoint,
            MAX_PATH))
        {
            LPWSTR VolumeName = reinterpret_cast<LPWSTR>(HeapAlloc(
                GetProcessHeap(),
                0,
                MAX_PATH * sizeof(wchar_t)));
            if (VolumeName)
            {
                if (GetVolumeNameForVolumeMountPointW(
                    VolumeMountPoint,
                    VolumeName,
                    MAX_PATH))
                {
                    LPWSTR LastChar = VolumeName;
                    do
                    {
                        ++LastChar;
                    } while (*LastChar);
                    --LastChar;

                    if (*LastChar == '\\')
                    {
                        *LastChar = '\0';
                    }

                    hVolume = CreateFileW(
                        VolumeName,
                        (NeedToWrite ? GENERIC_WRITE : GENERIC_READ),
                        FILE_SHARE_READ |FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        nullptr,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr);
                }

                HeapFree(GetProcessHeap(), 0, VolumeName);
            }
        }

        HeapFree(GetProcessHeap(), 0, VolumeMountPoint);
    }

    return hVolume;
}

struct FSCTL_REMOVE_OVERLAY_INPUT_BUFFER
{
    WOF_EXTERNAL_INFO WIMExternalInfo;
    WIM_PROVIDER_REMOVE_OVERLAY_INPUT WIMProviderRemoveOverlayInput;
};

HRESULT WINAPI WofWimRemoveEntry(
    __in PCWSTR VolumeName,
    __in LARGE_INTEGER DataSourceId)
{
    if (!VolumeName)
        return E_INVALIDARG;

    HANDLE hVolume = WofpOpenVolumeWithFlagsAndAttributes(VolumeName, TRUE);
    if (hVolume == INVALID_HANDLE_VALUE)
        return __HRESULT_FROM_WIN32(GetLastError());

    FSCTL_REMOVE_OVERLAY_INPUT_BUFFER InBuffer;
    DWORD BytesReturned;

    InBuffer.WIMExternalInfo.Version = WOF_CURRENT_VERSION;
    InBuffer.WIMExternalInfo.Provider = WOF_PROVIDER_WIM;
    InBuffer.WIMProviderRemoveOverlayInput.DataSourceId = DataSourceId;

    HRESULT hr = WofpDeviceIoControl(
        hVolume,
        FSCTL_REMOVE_OVERLAY,
        &InBuffer,
        sizeof(FSCTL_REMOVE_OVERLAY_INPUT_BUFFER),
        nullptr,
        0,
        &BytesReturned);

    CloseHandle(hVolume);

    return hr;
}

struct FSCTL_SUSPEND_OVERLAY_INPUT_BUFFER
{
    WOF_EXTERNAL_INFO WIMExternalInfo;
    WIM_PROVIDER_SUSPEND_OVERLAY_INPUT WIMProviderSuspendOverlayInput;
};

HRESULT WINAPI WofWimSuspendEntry(
    __in PCWSTR VolumeName,
    __in LARGE_INTEGER DataSourceId)
{
    if (!VolumeName)
        return E_INVALIDARG;

    HANDLE hVolume = WofpOpenVolumeWithFlagsAndAttributes(VolumeName, TRUE);
    if (hVolume == INVALID_HANDLE_VALUE)
        return __HRESULT_FROM_WIN32(GetLastError());

    FSCTL_SUSPEND_OVERLAY_INPUT_BUFFER InBuffer;
    DWORD BytesReturned;

    InBuffer.WIMExternalInfo.Version = WOF_CURRENT_VERSION;
    InBuffer.WIMExternalInfo.Provider = WOF_PROVIDER_WIM;
    InBuffer.WIMProviderSuspendOverlayInput.DataSourceId = DataSourceId;

    HRESULT hr = WofpDeviceIoControl(
        hVolume,
        FSCTL_SUSPEND_OVERLAY,
        &InBuffer,
        sizeof(FSCTL_SUSPEND_OVERLAY_INPUT_BUFFER),
        nullptr,
        0,
        &BytesReturned);

    CloseHandle(hVolume);

    return hr;
}


