//dllmain.cpp
#include "pch.h"
#include <Windows.h>
#include "ClassFactory.h"
#include "StrokeIME_GUID.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //MessageBox(NULL, L"StrokeIME.dll loaded", L"DllMain", MB_OK);
        OutputDebugString(L"[StrokeIME] DLL 已載入\n");
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugString(L"[StrokeIME] DLL 卸載\n");
        break;
    }
    return TRUE;
}

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    //MessageBox(NULL, L"DllGetClassObject called", L"Debug", MB_OK);

    if (rclsid != CLSID_StrokeIME)
        return CLASS_E_CLASSNOTAVAILABLE;

    ClassFactory* factory = new ClassFactory();
    
    return factory->QueryInterface(riid, ppv);
}