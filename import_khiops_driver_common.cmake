set(KhiopsDriverCommonSources
    checks.hpp
    checks.cpp
    contrib.hpp
    driver.h
    filestream_management.hpp
    globalstate.hpp
    logging.hpp
    logging.cpp
    returnval.hpp
    stringify.hpp
    userfunc_checks.hpp
    userfunc_checks.cpp
    util.hpp
    util.cpp
)
list(TRANSFORM KhiopsDriverCommonSources PREPEND src/khiops_driver_common/)