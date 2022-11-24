#pragma once 

#include <dlfcn.h>
#include <stdexcept>
#include "device.hpp"

using factory_error = std::runtime_error;

class factory {
    public:
        factory(const char * filename);
        Device * make_dev;
    private:
        void * handle;
        template<typename T>
        T load(const char * symbol) const {
            static_cast<void>(dlerror()); // clear errors
            return reinterpret_cast<T>(dlsym(handle, symbol));
        }
};
