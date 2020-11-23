//
// Created by grant on 11/19/20.
//

#pragma once

#ifndef C_SMART_PTR_INCLUDED
#define C_SMART_PTR_INCLUDED

#include <functional>

template <typename T>
class CSmartPtr {
public:
    T *val;
    std::function<void(T *in)> destructionFunc;

    CSmartPtr(T *n, std::function<void(T *in)> destroyer) : val(n), destructionFunc(destroyer) {}

    virtual ~CSmartPtr() {
        if (val != nullptr) {
            destructionFunc(val);
        }
    }
};


class Quitter {
public:
    virtual ~Quitter() {
        stms::consumeLogs();
        SDL_Quit();
    }
};

#endif