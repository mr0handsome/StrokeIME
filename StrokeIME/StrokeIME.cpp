//StrokeIME.cpp:
#include "pch.h"
#include "StrokeIME.h"
#include <cwchar>

StrokeIME::StrokeIME()
    : _refCount(1),
    _pThreadMgr(nullptr),
    _clientId(TF_CLIENTID_NULL),
    _pDocMgr(nullptr) {
}

STDMETHODIMP StrokeIME::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;
    *ppvObject = nullptr;

    if (riid == IID_IUnknown || riid == IID_ITfTextInputProcessor) {
        *ppvObject = static_cast<ITfTextInputProcessor*>(this);
    }
    else if (riid == IID_ITfKeyEventSink) {
        *ppvObject = static_cast<ITfKeyEventSink*>(this);
    }
    else if (riid == IID_ITfCompositionSink) {   // 必須要有這行，否則 StartComposition 會失敗
        *ppvObject = static_cast<ITfCompositionSink*>(this);
    }
    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) StrokeIME::AddRef() {
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(ULONG) StrokeIME::Release() {
    ULONG count = InterlockedDecrement(&_refCount);
    if (count == 0) delete this;
    return count;
}

STDMETHODIMP StrokeIME::Activate(ITfThreadMgr* pThreadMgr, TfClientId clientId) {
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();
    _clientId = clientId;

    // 建立 DocumentMgr 並保存到成員變數
    ITfDocumentMgr* pDocMgr = nullptr;
    if (SUCCEEDED(_pThreadMgr->CreateDocumentMgr(&pDocMgr))) {
        _pDocMgr = pDocMgr;   // 保存到成員
        _pDocMgr->AddRef();   // 增加引用計數
        pDocMgr->Release();   // 釋放區域變數的引用
    }

    // 註冊 KeyEventSink
    ITfKeystrokeMgr* pKeyMgr = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeyMgr))) {
        pKeyMgr->AdviseKeyEventSink(_clientId, static_cast<ITfKeyEventSink*>(this), TRUE);
        pKeyMgr->Release();
    }

    return S_OK;
}
STDMETHODIMP StrokeIME::OnSetFocus(BOOL fForeground) {
    return S_OK;
}

STDMETHODIMP StrokeIME::OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    wchar_t buf[128];
    swprintf_s(buf, L"[StrokeIME] OnTestKeyDown: VK=%d\n", (int)wParam);
    OutputDebugString(buf);

    *pfEaten = FALSE;
    return S_OK;
}


STDMETHODIMP StrokeIME::OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

/*STDMETHODIMP StrokeIME::OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    wchar_t buf[128];
    swprintf_s(buf, L"[StrokeIME] OnKeyDown: VK=%d\n", (int)wParam);
    OutputDebugString(buf);

    *pfEaten = FALSE; // 設為 TRUE 表示輸入法吃掉此按鍵
    return S_OK;
}*/


STDMETHODIMP StrokeIME::OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

STDMETHODIMP StrokeIME::OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}
STDMETHODIMP StrokeIME::Deactivate() {
    if (_pThreadMgr) {
        ITfKeystrokeMgr* pKeyMgr = nullptr;
        if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeyMgr))) {
            pKeyMgr->UnadviseKeyEventSink(_clientId);
            pKeyMgr->Release();
        }

        _pThreadMgr->Release();
        _pThreadMgr = nullptr;
    }
    if (_pDocMgr) {
        _pDocMgr->Release();
        _pDocMgr = nullptr;
    }

    _clientId = TF_CLIENTID_NULL;
    return S_OK;
}
// 1. 定義一個簡單的 EditSession 類別
class CEditSession : public ITfEditSession {
public:
    // 增加一個 wchar_t ch 參數
    CEditSession(StrokeIME* pTextService, ITfContext* pContext, wchar_t ch)
        : _refCount(1), _pTextService(pTextService), _pContext(pContext), _char(ch) {
        _pTextService->AddRef();
        _pContext->AddRef();
    }
    ~CEditSession() { _pTextService->Release(); _pContext->Release(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_ITfEditSession) {
            *ppv = static_cast<ITfEditSession*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_refCount); }
    STDMETHODIMP_(ULONG) Release() {
        ULONG res = InterlockedDecrement(&_refCount);
        if (res == 0) delete this;
        return res;
    }

    // 這裡修正：呼叫 _pTextService 內部的處理邏輯
    STDMETHODIMP DoEditSession(TfEditCookie ec) {
        _pTextService->_InternalOnKeyDown(ec, _pContext, _char);
        return S_OK;
    }

private:
    LONG _refCount;
    StrokeIME* _pTextService;
    ITfContext* _pContext;
    wchar_t _char; // 儲存按鍵
};
void StrokeIME::_InternalOnKeyDown(TfEditCookie ec, ITfContext* pic, wchar_t ch) {
    // 1. 更新筆劃緩衝區
    _strokeBuffer += ch;

    // 2. 處理組合字串 (Composition)
    if (_pComposition == nullptr) {
        // 如果還沒開始，先建立插入點
        ITfInsertAtSelection* pInsertAtSelection = nullptr;
        if (SUCCEEDED(pic->QueryInterface(IID_ITfInsertAtSelection, (void**)&pInsertAtSelection))) {
            ITfRange* pRange = nullptr;
            // 使用正確的 ec (Edit Cookie)
            if (SUCCEEDED(pInsertAtSelection->InsertTextAtSelection(ec, 0, NULL, 0, &pRange))) {
                ITfContextComposition* pContextComposition = nullptr;
                if (SUCCEEDED(pic->QueryInterface(IID_ITfContextComposition, (void**)&pContextComposition))) {
                    // 開始組合
                    pContextComposition->StartComposition(ec, pRange, this, &_pComposition);
                    pContextComposition->Release();
                }

                if (_pComposition != nullptr) {
                    // 在畫面上顯示筆劃 (例如 "1")
                    pRange->SetText(ec, 0, _strokeBuffer.c_str(), (LONG)_strokeBuffer.length());
                }
                pRange->Release();
            }
            pInsertAtSelection->Release();
        }
    }
    else {
        // 已經在組合中，直接更新文字
        ITfRange* pRange = nullptr;
        if (SUCCEEDED(_pComposition->GetRange(&pRange))) {
            pRange->SetText(ec, 0, _strokeBuffer.c_str(), (LONG)_strokeBuffer.length());
            pRange->Release();
        }
    }
}
STDMETHODIMP StrokeIME::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition) {
    if (_pComposition) {
        _pComposition->Release();
        _pComposition = nullptr;
    }
    _strokeBuffer.clear();
    return S_OK;
}
void StrokeIME::_TerminateComposition() {
    if (_pComposition != nullptr) {
        _pComposition->Release();
        _pComposition = nullptr;
    }
    _strokeBuffer.clear(); // 清空目前的筆劃記錄
}
STDMETHODIMP StrokeIME::OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    if (wParam >= '1' && wParam <= '5') {
        *pfEaten = TRUE;

        // 關鍵修正處：增加第三個引數 (wchar_t)wParam
        CEditSession* pEditSession = new CEditSession(this, pic, (wchar_t)wParam);

        if (pEditSession) {
            HRESULT hr;
            // 請求一個同步編輯工作階段
            pic->RequestEditSession(_clientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
            pEditSession->Release();
        }

        return S_OK;
    }
    *pfEaten = FALSE;
    return S_OK;
}