// added by iOrange in 2024

#pragma once

// simplified version of Microsoft::WRL::ComPtr

template <typename T>
class IUnknownPtr {
public:
    IUnknownPtr() : mPtr(nullptr) {}

    ~IUnknownPtr() {
        InternalRelease();
    }

    IUnknownPtr& operator=(decltype(__nullptr)) {
        InternalRelease();
        return *this;
    }

    IUnknownPtr& operator=(T* other) {
        if (mPtr != other) {
            InternalRelease();
            mPtr = other;
        }
        return *this;
    }

    operator bool() const {
        return mPtr != nullptr;
    }

    T* GetPtr() const {
        return mPtr;
    }

    T* operator->() const {
        return mPtr;
    }

    T* const* GetAddressOf() const {
        return &mPtr;
    }

    T** GetAddressOf() {
        return &mPtr;
    }

    T** ReleaseAndGetAddressOf() {
        InternalRelease();
        return &mPtr;
    }

private:
    void InternalAddRef() const {
        if (mPtr) {
            mPtr->AddRef();
        }
    }

    unsigned long InternalRelease() {
        unsigned long refs = 0ul;
        T* temp = mPtr;

        if (temp) {
            mPtr = nullptr;
            refs = temp->Release();
        }

        return refs;
    }

private:
    T*  mPtr;
};
