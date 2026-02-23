//ClassFactory.h
#pragma once
#include <Windows.h>
#include <Unknwn.h> // IUnknown
#include <objbase.h> // IClassFactory

class ClassFactory : public IClassFactory {
public:
    ClassFactory();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);

private:
    LONG _refCount;
};