/*
 * PROJECT:   WofUtilDemo
 * FILE:      WofUtil.cpp
 * PURPOSE:   Reverse Engineering of WofUtil.dll
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#include "pch.h"

#include <Windows.h>
#include <winioctl.h>

#include "ChuyuWindowsInternalAPI.h"

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
WofSetFileDataLocation - Reversed
WofShouldCompressBinaries
WofShouldCompressBinariesEx
WofStoreEvalData
WofWimAddEntry
WofWimEnumFiles
WofWimRemoveEntry - Reversed
WofWimSuspendEntry - Reversed
WofWimUpdateEntry - Reversed
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

        return __HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
    }

    return  __HRESULT_FROM_WIN32(GetLastError());
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

HRESULT WINAPI WofSetFileDataLocation(
    __in HANDLE FileHandle,
    __in ULONG Provider,
    __in PVOID ExternalFileInfo,
    __in ULONG Length)
{
    bool IsInvalidArg = false;

    if (Provider == WOF_PROVIDER_WIM)
        IsInvalidArg = Length < sizeof(WIM_EXTERNAL_FILE_INFO_V0);
    else if (Provider == WOF_PROVIDER_FILE)
        IsInvalidArg = Length < sizeof(WOF_FILE_COMPRESSION_INFO_V0);
    else
        IsInvalidArg = true;

    if (IsInvalidArg)
        return E_INVALIDARG;

    struct
    {
        WOF_EXTERNAL_INFO WofInfo;
        union
        {
            WIM_PROVIDER_EXTERNAL_INFO Wim;
            FILE_PROVIDER_EXTERNAL_INFO File;
        } ProviderExternalInfo;
    } InBuffer = { 0 };
    DWORD lpNumberOfBytesTransferred = sizeof(WOF_EXTERNAL_INFO);
    OVERLAPPED Overlapped = { 0 };

    InBuffer.WofInfo.Version = WOF_CURRENT_VERSION;

    if (Provider == WOF_PROVIDER_WIM)
    {
        InBuffer.WofInfo.Provider = WOF_PROVIDER_WIM;

        PWIM_EXTERNAL_FILE_INFO_V0 pWimExternalFileInfo =
            reinterpret_cast<PWIM_EXTERNAL_FILE_INFO_V0>(ExternalFileInfo);

        InBuffer.ProviderExternalInfo.Wim.Version =
            WIM_PROVIDER_CURRENT_VERSION;
        InBuffer.ProviderExternalInfo.Wim.Flags =
            0;
        InBuffer.ProviderExternalInfo.Wim.DataSourceId =
            pWimExternalFileInfo->DataSourceId;
        memcpy(
            InBuffer.ProviderExternalInfo.Wim.ResourceHash,
            pWimExternalFileInfo->ResourceHash,
            sizeof(pWimExternalFileInfo->ResourceHash));

        lpNumberOfBytesTransferred += sizeof(WIM_PROVIDER_EXTERNAL_INFO);
    }
    else if(Provider == WOF_PROVIDER_FILE)
    {
        InBuffer.WofInfo.Provider = WOF_PROVIDER_FILE;

        PWOF_FILE_COMPRESSION_INFO_V0 pFileCompressionInfo =
            reinterpret_cast<PWOF_FILE_COMPRESSION_INFO_V0>(ExternalFileInfo);

        InBuffer.ProviderExternalInfo.File.Version =
            FILE_PROVIDER_CURRENT_VERSION;
        InBuffer.ProviderExternalInfo.File.Algorithm =
            pFileCompressionInfo->Algorithm;
        InBuffer.ProviderExternalInfo.File.Flags =
            0;

        lpNumberOfBytesTransferred += sizeof(FILE_PROVIDER_EXTERNAL_INFO);
    }

    Overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!Overlapped.hEvent)
        return __HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES);

    bool IsSucceed = DeviceIoControl(
        FileHandle,
        FSCTL_SET_EXTERNAL_BACKING,
        &InBuffer,
        lpNumberOfBytesTransferred,
        nullptr,
        0,
        &lpNumberOfBytesTransferred,
        &Overlapped);

    if (!IsSucceed)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            IsSucceed = GetOverlappedResult(
                FileHandle,
                &Overlapped,
                &lpNumberOfBytesTransferred,
                TRUE);
        }
    }

    HRESULT hr = IsSucceed ? S_OK : __HRESULT_FROM_WIN32(GetLastError());

    CloseHandle(Overlapped.hEvent);

    return hr;
}

struct FSCTL_UPDATE_OVERLAY_INPUT_BUFFER
{
    WOF_EXTERNAL_INFO WIMExternalInfo;
    WIM_PROVIDER_UPDATE_OVERLAY_INPUT WIMProviderUpdateOverlayInput;
};

HRESULT WINAPI WofWimUpdateEntry(
    __in PCWSTR VolumeName,
    __in LARGE_INTEGER DataSourceId,
    __in PCWSTR NewWimPath)
{
    if (!VolumeName || !NewWimPath)
        return E_INVALIDARG;

    UNICODE_STRING NewWimNtPath = { 0 };
    HRESULT hr = __HRESULT_FROM_WIN32(RtlNtStatusToDosError(
        RtlDosPathNameToNtPathName_U_WithStatus(
            const_cast<PWSTR>(NewWimPath),
            &NewWimNtPath,
            nullptr,
            nullptr)));
    if (SUCCEEDED(hr))
    {
        DWORD InBufferSize = NewWimNtPath.Length + 24;
        FSCTL_UPDATE_OVERLAY_INPUT_BUFFER* InBuffer =
            reinterpret_cast<FSCTL_UPDATE_OVERLAY_INPUT_BUFFER*>(
                HeapAlloc(GetProcessHeap(), 0, InBufferSize));
        if (InBuffer)
        {
            HANDLE hVolume = WofpOpenVolumeWithFlagsAndAttributes(
                VolumeName, TRUE);
            if (hVolume != INVALID_HANDLE_VALUE)
            {
                InBuffer->WIMExternalInfo.Version =
                    WOF_CURRENT_VERSION;
                InBuffer->WIMExternalInfo.Provider =
                    WOF_PROVIDER_WIM;
                InBuffer->WIMProviderUpdateOverlayInput.DataSourceId =
                    DataSourceId;
                InBuffer->WIMProviderUpdateOverlayInput.WimFileNameOffset =
                    16;
                InBuffer->WIMProviderUpdateOverlayInput.WimFileNameLength =
                    NewWimNtPath.Length;
                memcpy(&InBuffer[1], NewWimNtPath.Buffer, NewWimNtPath.Length);

                DWORD BytesReturned;
                hr = WofpDeviceIoControl(
                    hVolume,
                    FSCTL_UPDATE_OVERLAY,
                    InBuffer,
                    InBufferSize,
                    nullptr,
                    0,
                    &BytesReturned);

                CloseHandle(hVolume);
            }
            else
            {
                hr = __HRESULT_FROM_WIN32(GetLastError());
            }


            HeapFree(GetProcessHeap(), 0, InBuffer);
        }

        HeapFree(GetProcessHeap(), 0, NewWimNtPath.Buffer);
    }

    return hr;
}
