set(KhiopsDriverCommonSources
    backend.hpp
    contrib.hpp
    driver.h
    driver.cpp
    filestream.hpp
    filestream.cpp
    logging.hpp
    logging.cpp
    returnval.hpp
    stringify.hpp
    util.hpp
    util.cpp
)
list(TRANSFORM KhiopsDriverCommonSources PREPEND src/khiops_driver_common/)