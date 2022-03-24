find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_INSPECTOR gnuradio-inspector)

FIND_PATH(
    GR_INSPECTOR_INCLUDE_DIRS
    NAMES gnuradio/inspector/api.h
    HINTS $ENV{INSPECTOR_DIR}/include
        ${PC_INSPECTOR_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_INSPECTOR_LIBRARIES
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

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-inspectorTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_INSPECTOR DEFAULT_MSG GR_INSPECTOR_LIBRARIES GR_INSPECTOR_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_INSPECTOR_LIBRARIES GR_INSPECTOR_INCLUDE_DIRS)
