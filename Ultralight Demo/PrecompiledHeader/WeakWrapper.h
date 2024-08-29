#pragma once
#include <iostream>
#include <memory>

template<typename T>
class WeakWrapper 
{
private:
    std::weak_ptr<T> weakPtr;

public:
    WeakWrapper() {}
    // Constructor to initialize from a shared_ptr
    WeakWrapper(const std::shared_ptr<T>& sharedPtr) : weakPtr(sharedPtr) {}

    // Constructor to initialize from a weak_ptr
    WeakWrapper(const std::weak_ptr<T>& weakPtr) : weakPtr(weakPtr) {}

    // Overloading the -> operator to automatically lock the weak_ptr
    T* operator->() {
        auto sharedPtr = weakPtr.lock();
        if (!sharedPtr) {
            throw std::runtime_error("Accessing expired weak_ptr");
        }
        return sharedPtr.get();
    }

    T* get() {
        std::shared_ptr<T> sharedPtr = weakPtr.lock();
        return sharedPtr ? sharedPtr.get() : nullptr;
    }

    // Provide access to the weak_ptr, in case manual locking is needed
    std::weak_ptr<T> getWeakPtr() const {
        return weakPtr;
    }

    // Check if the weak_ptr is expired
    bool expired() const {
        return weakPtr.expired();
    }

    // Comparison operator to compare with std::shared_ptr
    bool operator==(const std::shared_ptr<T>& otherSharedPtr) const {
        // Lock the weak_ptr to get a shared_ptr
        auto lockedPtr = weakPtr.lock();
        // Compare raw pointers (get() returns nullptr if expired)
        return lockedPtr.get() == otherSharedPtr.get();
    }

    // Comparison operator to compare with another WeakWrapper
    bool operator==(const WeakWrapper<T>& other) const {
        auto thisLocked = weakPtr.lock();
        auto otherLocked = other.weakPtr.lock();
        return thisLocked.get() == otherLocked.get();
    }
};