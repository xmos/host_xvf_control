#include "factory.hpp"

factory::factory(const char * filename) : handle(dlopen(filename, RTLD_NOW | RTLD_LOCAL))
{
    if (handle == NULL) throw factory_error(std::string(dlerror()));
    make_dev = load<Device *>("make_Dev");
}
