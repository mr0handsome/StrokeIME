#include <windows.h>
#include <msctf.h>
#include <objbase.h>
#include <iostream>

// 宣告你的 CLSID（與 DLL 中一致）
const CLSID CLSID_StrokeIME =
{ 0x734fd453, 0xa4f8, 0x4c5d, { 0x9c, 0x98, 0x3f, 0xe2, 0xd7, 0x07, 0x97, 0x60 } };

int main() {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cout << "CoInitialize 失敗\n";
        return 1;
    }

    ITfInputProcessorProfiles* pProfiles = nullptr;
    GUID guidProfile = { 0xb6a1f3e2, 0x9d4c, 0x4f7a, { 0x8c, 0x3d, 0x1e, 0x2b, 0x5a, 0x7c, 0x9f, 0x01 } };

    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles, (void**)&pProfiles)))
    {
        pProfiles->Register(CLSID_StrokeIME);

        hr = pProfiles->AddLanguageProfile(
            CLSID_StrokeIME,
            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG),
            guidProfile,
            L"筆劃輸入法",
            (UINT)wcslen(L"筆劃輸入法"),
            L"", 0, 0);

        if (SUCCEEDED(hr)) {
            std::wcout << L"AddLanguageProfile 成功！\n";
        }
        else {
            std::wcout << L"AddLanguageProfile falu：" << std::hex << hr << L"\n";
        }

        pProfiles->Release();
    }
    else {
        std::cout << "無法建立 ITfInputProcessorProfiles 實例\n";
    }
    return 0;
}