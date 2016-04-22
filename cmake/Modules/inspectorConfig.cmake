INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_INSPECTOR inspector)

FIND_PATH(
    INSPECTOR_INCLUDE_DIRS
    NAMES inspector/api.h
    HINTS $ENV{INSPECTOR_DIR}/include
        ${PC_INSPECTOR_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    INSPECTOR_LIBRARIES
    NAMES gnuradio-inspector
    HINTS $ENV{INSPECTOR_DIR}/lib
        ${PC_INSPECTOR_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(INSPECTOR DEFAULT_MSG INSPECTOR_LIBRARIES INSPECTOR_INCLUDE_DIRS)
MARK_AS_ADVANCED(INSPECTOR_LIBRARIES INSPECTOR_INCLUDE_DIRS)

