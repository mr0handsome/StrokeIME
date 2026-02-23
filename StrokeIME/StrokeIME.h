#pragma once
// StrokeIME.h
#include <windows.h>
#include <msctf.h>
#include <string>

// 繼承 ITfTextInputProcessor (基礎介面)
// 繼承 ITfKeyEventSink (處理鍵盤事件)
// 繼承 ITfCompositionSink (處理文字組合狀態)
class StrokeIME : public ITfTextInputProcessor,
    public ITfKeyEventSink,
    public ITfCompositionSink {
public:
    StrokeIME();

    // IUnknown 介面實作
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // ITfTextInputProcessor 介面實作
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId clientId);
    STDMETHODIMP Deactivate();
    
    // ITfKeyEventSink 介面實作
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten);

    // ITfCompositionSink 介面實作 (當組合被系統中止時觸發)
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition);

    // 內部邏輯：由 CEditSession 呼叫，提供正確的 TfEditCookie
    void _InternalOnKeyDown(TfEditCookie ec, ITfContext* pic, wchar_t ch);

private:
    // COM 引用計數
    ULONG _refCount;

    // TSF 核心成員
    ITfThreadMgr* _pThreadMgr;
    TfClientId _clientId;
    ITfDocumentMgr* _pDocMgr;

    // 組合字串管理
    ITfComposition* _pComposition; // 目前正在輸入的組合
    std::wstring _strokeBuffer;    // 筆劃暫存 (例如 "135")

    // 私有清理函數
    void _TerminateComposition();
};