//Register.cpp
#include "pch.h"
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#include "StrokeIME_GUID.h"

#pragma comment(lib, "Shlwapi.lib")

#ifndef SELFREG_E_CLASS
#define SELFREG_E_CLASS HRESULT_FROM_WIN32(ERROR_CLASS_DOES_NOT_EXIST)
#endif

extern "C" IMAGE_DOS_HEADER __ImageBase;

// COM 註冊：DllRegisterServer
extern "C" __declspec(dllexport) HRESULT __stdcall DllRegisterServer() {
    MessageBox(NULL, L"開始執行 DllRegisterServer", L"Debug", MB_OK);
    WCHAR szCLSID[64], szSubkey[MAX_PATH], szModulePath[MAX_PATH];
    HKEY hKey = nullptr;

    // CLSID → 字串
    if (StringFromGUID2(CLSID_StrokeIME, szCLSID, ARRAYSIZE(szCLSID)) == 0) {
        MessageBox(NULL, L"CLSID 轉換失敗", L"DllRegisterServer", MB_OK);
        return SELFREG_E_CLASS;
    }

    // 組合註冊表路徑
    HRESULT hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    if (FAILED(hr)) {
        MessageBox(NULL, L"註冊表路徑格式化失敗", L"DllRegisterServer", MB_OK);
        return SELFREG_E_CLASS;
    }

    // 建立 CLSID 主鍵
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szSubkey, 0, nullptr,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS) {
        MessageBox(NULL, L"建立 CLSID 主鍵失敗", L"DllRegisterServer", MB_OK);
        return SELFREG_E_CLASS;
    }

    // 建立 InprocServer32 子鍵
    HKEY hInproc = nullptr;
    if (RegCreateKeyEx(hKey, L"InprocServer32", 0, nullptr,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hInproc, nullptr) != ERROR_SUCCESS) {
        MessageBox(NULL, L"建立 InprocServer32 子鍵失敗", L"DllRegisterServer", MB_OK);
        RegCloseKey(hKey);
        return SELFREG_E_CLASS;
    }

    // 取得 DLL 路徑
    DWORD len = GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), szModulePath, ARRAYSIZE(szModulePath));
    if (len == 0 || len >= ARRAYSIZE(szModulePath)) {
        MessageBox(NULL, L"取得 DLL 路徑失敗", L"DllRegisterServer", MB_OK);
        RegCloseKey(hInproc);
        RegCloseKey(hKey);
        return SELFREG_E_CLASS;
    }

    // 寫入 DLL 路徑
    if (RegSetValueEx(hInproc, nullptr, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(szModulePath),
        static_cast<DWORD>((wcslen(szModulePath) + 1) * sizeof(WCHAR))) != ERROR_SUCCESS) {
        MessageBox(NULL, L"寫入 DLL 路徑失敗", L"DllRegisterServer", MB_OK);
        RegCloseKey(hInproc);
        RegCloseKey(hKey);
        return SELFREG_E_CLASS;
    }

    // 寫入 ThreadingModel
    if (RegSetValueEx(hInproc, L"ThreadingModel", 0, REG_SZ,
        reinterpret_cast<const BYTE*>(L"Apartment"),
        static_cast<DWORD>((wcslen(L"Apartment") + 1) * sizeof(WCHAR))) != ERROR_SUCCESS) {
        MessageBox(NULL, L"寫入 ThreadingModel 失敗", L"DllRegisterServer", MB_OK);
        RegCloseKey(hInproc);
        RegCloseKey(hKey);
        return SELFREG_E_CLASS;
    }

    RegCloseKey(hInproc);
    RegCloseKey(hKey);

    MessageBox(NULL, L"DllRegisterServer 成功！", L"註冊完成", MB_OK);
    return S_OK;
}

// COM 解除註冊：DllUnregisterServer
extern "C" __declspec(dllexport) HRESULT __stdcall DllUnregisterServer() {
    WCHAR szCLSID[64], szSubkey[MAX_PATH];

    if (StringFromGUID2(CLSID_StrokeIME, szCLSID, ARRAYSIZE(szCLSID)) == 0) {
        MessageBox(NULL, L"CLSID 轉換失敗", L"DllUnregisterServer", MB_OK);
        return SELFREG_E_CLASS;
    }

    HRESULT hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    if (FAILED(hr)) {
        MessageBox(NULL, L"註冊表路徑格式化失敗", L"DllUnregisterServer", MB_OK);
        return SELFREG_E_CLASS;
    }

    if (SHDeleteKey(HKEY_CLASSES_ROOT, szSubkey) != ERROR_SUCCESS) {
        MessageBox(NULL, L"刪除 CLSID 註冊表失敗", L"DllUnregisterServer", MB_OK);
        return SELFREG_E_CLASS;
    }

    MessageBox(NULL, L"DllUnregisterServer 成功！", L"解除註冊完成", MB_OK);
    return S_OK;
}