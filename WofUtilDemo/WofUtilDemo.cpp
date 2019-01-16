// WofUtilDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include <Windows.h>

#include "wofapi_sdk.h"



int main()
{
    wchar_t x[MAX_PATH];

    GetModuleFileNameW(nullptr, x, MAX_PATH);


    HANDLE hFile = CreateFileW(
        x,
        FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr);


    struct
    {
        uint16_t Build;
        uint8_t Minor;
        uint8_t Major;

    } y;


    ULONG* WofVersion = (ULONG*)&y;

    auto z = WofGetDriverVersion(hFile, WOF_PROVIDER_CLOUD, WofVersion);

    return 0;
}
