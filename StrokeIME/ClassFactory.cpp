//Classfactory.cpp
#include "pch.h"
#include "ClassFactory.h"
#include "StrokeIME.h"
#include <new>
#include <msctf.h>

ClassFactory::ClassFactory() : _refCount(1) {}

// IUnknown
STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;
    *ppvObject = nullptr;

    if (riid == IID_IUnknown || riid == IID_IClassFactory) {
        *ppvObject = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef() {
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(ULONG) ClassFactory::Release() {
    ULONG count = InterlockedDecrement(&_refCount);
    if (count == 0) delete this;
    return count;
}

// IClassFactory
STDMETHODIMP ClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) {
    OutputDebugString(L"[StrokeIME] ClassFactory::CreateInstance called\n");

    if (!ppvObject) return E_POINTER;
    *ppvObject = nullptr;

    if (pUnkOuter != nullptr) return CLASS_E_NOAGGREGATION;

    StrokeIME* pStrokeIME = new (std::nothrow) StrokeIME();
    if (!pStrokeIME) return E_OUTOFMEMORY;

    HRESULT hr = pStrokeIME->QueryInterface(riid, ppvObject);
    if (FAILED(hr)) {
        OutputDebugString(L"[StrokeIME] CreateInstance: QueryInterface failed\n");
        delete pStrokeIME;
        return hr;
    }

    return S_OK;
}

STDMETHODIMP ClassFactory::LockServer(BOOL fLock) {
    OutputDebugString(L"[StrokeIME] ClassFactory::LockServer called\n");
    return S_OK;
}