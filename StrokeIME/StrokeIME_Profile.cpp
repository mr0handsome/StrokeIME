// StrokeIME_Profile.cpp
#include "pch.h"
#include <windows.h>
#include <msctf.h>
#include <iostream>
#include "StrokeIME_GUID.h" // Contains CLSID_StrokeIME declaration

// Profile GUIDs must be unique
int main() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::wcout << L"CoInitialize failed, HRESULT = 0x" << std::hex << hr << std::endl;
        return 1;
    }

    ITfInputProcessorProfiles* pProfiles = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles, (void**)&pProfiles);

    if (FAILED(hr)) {
        std::wcout << L"Failed to create ITfInputProcessorProfiles instance, HRESULT = 0x" << std::hex << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    // Register COM CLSID
    hr = pProfiles->Register(CLSID_StrokeIME);
    if (FAILED(hr)) {
        std::wcout << L"Register CLSID failed, HRESULT = 0x" << std::hex << hr << std::endl;
        pProfiles->Release();
        CoUninitialize();
        return 1;
    }

    // Taiwan Traditional Chinese Profile
    LANGID langTW = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL);
    hr = pProfiles->AddLanguageProfile(
        CLSID_StrokeIME,
        langTW,
        GUID_Profile_TW,
        L"Stroke IME (Taiwan)",
        (ULONG)wcslen(L"Stroke IME (Taiwan)"),
        nullptr, 0, NULL // No icon or tooltip
    );
    if (SUCCEEDED(hr)) {
        pProfiles->EnableLanguageProfile(CLSID_StrokeIME, langTW, GUID_Profile_TW, TRUE);
        std::wcout << L"Successfully registered and enabled Taiwan Traditional Chinese profile." << std::endl;
    }
    else {
        std::wcout << L"Failed to register Taiwan profile, HRESULT = 0x" << std::hex << hr << std::endl;
    }
    // 在 main 函數中 AddLanguageProfile 之後執行：
    ITfCategoryMgr* pCategoryMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
    if (SUCCEEDED(hr)) {
        // 註冊為鍵盤 TIP
        hr = pCategoryMgr->RegisterCategory(CLSID_StrokeIME, GUID_TFCAT_TIP_KEYBOARD, CLSID_StrokeIME);

        // 如果是 Windows 8/10/11，建議也加入以下分類以確保在 UWP/鎖屏介面正常顯示
        // GUID_TFCAT_TIPCAP_SECUREMODE, GUID_TFCAT_TIPCAP_UIELEMENTENABLED 等
        pCategoryMgr->Release();
    }
    // Hong Kong Traditional Chinese Profile
    LANGID langHK = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG);
    hr = pProfiles->AddLanguageProfile(
        CLSID_StrokeIME,
        langHK,
        GUID_Profile_HK,
        L"Stroke IME (Hong Kong)",
        (ULONG)wcslen(L"Stroke IME (Hong Kong)"),
        nullptr, 0, NULL // No icon or tooltip
    );
    if (SUCCEEDED(hr)) {
        pProfiles->EnableLanguageProfile(CLSID_StrokeIME, langHK, GUID_Profile_HK, TRUE);
        std::wcout << L"Successfully registered and enabled Hong Kong Traditional Chinese profile." << std::endl;
    }
    else {
        std::wcout << L"Failed to register Hong Kong profile, HRESULT = 0x" << std::hex << hr << std::endl;
    }

    pProfiles->Release();
    CoUninitialize();
    return 0;
}