#include "woflib.h"

#include <Windows.h>

typedef struct _WOF_FILE_PROVIDER_INFO
{
	WOF_EXTERNAL_INFO WofInfo;
	FILE_PROVIDER_EXTERNAL_INFO FileProviderInfo;
} WOF_FILE_PROVIDER_INFO, *PWOF_FILE_PROVIDER_INFO;

// Get the compression algorithm that is used to compress a file with the 
// Windows Overlay Filter File Provider.
// Parameters:
//   FileHandle: A handle to the file on which the operation is to be 
//   performed. To retrieve a file handle, use the CreateFile function. 
//   FileCompressionAlgorithm: Returns the compression algorithm that is used
//   to compress this file. Currently defined algorithms are: 
//     FILE_PROVIDER_COMPRESSION_XPRESS4K: Indicates that the data for the file
//     should be compressed in 4kb chunks with the XPress algorithm. This 
//     algorithm is designed to be computationally lightweight, and provides 
//     for rapid access to data.
//	   FILE_PROVIDER_COMPRESSION_LZX: Indicates that the data for the file 
//     should be compressed in 32kb chunks with the LZX algorithm. This 
//     algorithm is designed to be highly compact, and provides for small 
//     footprint for infrequently accessed data.
//	   FILE_PROVIDER_COMPRESSION_XPRESS8K: Indicates that the data for the file
//     should be compressed in 8kb chunks with the XPress algorithm.
//	   FILE_PROVIDER_COMPRESSION_XPRESS16K: Indicates that the data for the 
//     file should be compressed in 16kb chunks with the XPress algorithm.
//     If the function fails, this parameter will return -1.
// Return value:
//   The function will return HRESULT. If the function succeeds, the return 
//   value is S_OK.
EXTERN_C HRESULT WINAPI ChuyuWOFQueryFileCompressionAlgorithm(
	_In_ HANDLE FileHandle,
	_Out_ PDWORD FileCompressionAlgorithm)
{
	*FileCompressionAlgorithm = static_cast<DWORD>(-1);
	
	WOF_FILE_PROVIDER_INFO FileProviderInfo;

	if (!DeviceIoControl(
		FileHandle,
		FSCTL_GET_EXTERNAL_BACKING,
		nullptr,
		0,
		&FileProviderInfo,
		sizeof(FileProviderInfo),
		nullptr,
		nullptr))
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}

	*FileCompressionAlgorithm = FileProviderInfo.FileProviderInfo.Algorithm;
	
	return S_OK;
}

// Compress a file with the Windows Overlay Filter File Provider.
// Parameters:
//   FileHandle: A handle to the file on which the operation is to be 
//   performed. To retrieve a file handle, use the CreateFile function. 
//   FileCompressionAlgorithm: Specifies the compression algorithm that is used
//   to compress this file. Currently defined algorithms are: 
//     FILE_PROVIDER_COMPRESSION_XPRESS4K: Indicates that the data for the file
//     should be compressed in 4kb chunks with the XPress algorithm. This 
//     algorithm is designed to be computationally lightweight, and provides 
//     for rapid access to data.
//	   FILE_PROVIDER_COMPRESSION_LZX: Indicates that the data for the file 
//     should be compressed in 32kb chunks with the LZX algorithm. This 
//     algorithm is designed to be highly compact, and provides for small 
//     footprint for infrequently accessed data.
//	   FILE_PROVIDER_COMPRESSION_XPRESS8K: Indicates that the data for the file
//     should be compressed in 8kb chunks with the XPress algorithm.
//	   FILE_PROVIDER_COMPRESSION_XPRESS16K: Indicates that the data for the 
//     file should be compressed in 16kb chunks with the XPress algorithm.
// Return value:
//   The function will return HRESULT. If the function succeeds, the return 
//   value is S_OK.
EXTERN_C HRESULT WINAPI ChuyuWOFCompressFile(
	_In_ HANDLE FileHandle,
	_In_ DWORD FileCompressionAlgorithm)
{
	WOF_FILE_PROVIDER_INFO FileProviderInfo;

	FileProviderInfo.WofInfo.Version = WOF_CURRENT_VERSION;
	FileProviderInfo.WofInfo.Provider = WOF_PROVIDER_FILE;
	FileProviderInfo.FileProviderInfo.Version = FILE_PROVIDER_CURRENT_VERSION;
	FileProviderInfo.FileProviderInfo.Algorithm = FileCompressionAlgorithm;
	FileProviderInfo.FileProviderInfo.Flags = 0;

	if (!DeviceIoControl(
		FileHandle,
		FSCTL_SET_EXTERNAL_BACKING,
		&FileProviderInfo,
		sizeof(FileProviderInfo),
		nullptr,
		0,
		nullptr,
		nullptr))
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}

// Uncompress a file with the Windows Overlay Filter File Provider.
// Parameters:
//   FileHandle: A handle to the file on which the operation is to be 
//   performed. To retrieve a file handle, use the CreateFile function. 
// Return value:
//   The function will return HRESULT. If the function succeeds, the return 
//   value is S_OK.
EXTERN_C HRESULT WINAPI ChuyuWOFUncompressFile(
	_In_ HANDLE FileHandle)
{
	if (!DeviceIoControl(
		FileHandle,
		FSCTL_DELETE_EXTERNAL_BACKING,
		nullptr,
		0,
		nullptr,
		0,
		nullptr,
		nullptr))
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}
