set(KhiopsDriverCommonSources
    backend.hpp
    contrib.hpp
    driver.h
    driver.cpp
    filestream.hpp
    filestream.cpp
    filestreamregistry.hpp
    filestreamregistry.cpp
    logging.hpp
    logging.cpp
    returnval.hpp
    stringify.hpp
    util.hpp
)
list(TRANSFORM KhiopsDriverCommonSources PREPEND src/khiops_driver_common/)