#pragma once

#ifndef _WOFLIB_
#define _WOFLIB_

#include <Windows.h>

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
	_Out_ PDWORD FileCompressionAlgorithm);

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
	_In_ DWORD FileCompressionAlgorithm);

// Uncompress a file with the Windows Overlay Filter File Provider.
// Parameters:
//   FileHandle: A handle to the file on which the operation is to be 
//   performed. To retrieve a file handle, use the CreateFile function. 
// Return value:
//   The function will return HRESULT. If the function succeeds, the return 
//   value is S_OK.
EXTERN_C HRESULT WINAPI  ChuyuWOFUncompressFile(
	_In_ HANDLE FileHandle);

#endif // !_WOFLIB_
