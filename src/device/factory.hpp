// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef FACTORY_CLASS_H_
#define FACTORY_CLASS_H_

#include <dlfcn.h>
#include <stdexcept>
#include "device.hpp"

using factory_error = std::runtime_error;

class factory {
    public:
        factory(const char * filename);
        using device_t = std::unique_ptr<Device> (*)(void *);
        device_t make_dev;
    private:
        void * handle;
        template<typename T>
        T load(const char * symbol) const {
            static_cast<void>(dlerror()); // clear errors
            return reinterpret_cast<T>(dlsym(handle, symbol));
        }
};

#endif
