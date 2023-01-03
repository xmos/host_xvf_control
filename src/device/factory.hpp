// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef FACTORY_CLASS_H_
#define FACTORY_CLASS_H_

#include <dlfcn.h>
#include <stdexcept>
#include "device.hpp"

/** Function pointer that takes void * and returns unique_ptr<Device> */
using device_t = std::unique_ptr<Device> (*)(void *);

/**
 * @brief Class for dynamically opening device_* shared object 
 * and storing function pointer that returns unique_ptr<Device>
 */
class factory
{
    public:

        /**
         * @brief Construct a new factory object
         * 
         * @param filename  Name of the shared object to get the Device from
         */
        factory(const char * filename);

        /** @brief Function pointer that returns unique_ptr<Device> */
        device_t make_dev;

    private:

        /** @brief Pointer to the device_* shared object */
        void * handle;

        /**
         * @brief Loads function pointer from a handle
         * 
         * @tparam T        Function pointer type
         * @param symbol    Name of the function to look for
         */
        template<typename T>
        T load(const char * symbol) const {
            static_cast<void>(dlerror()); // clear errors
            return reinterpret_cast<T>(dlsym(handle, symbol));
        }
};

#endif
