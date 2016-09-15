# Based on https://github.com/w0land/thesis-app/blob/master/cmake/modules/FindQwtPlot3D.cmake

# - try to find QwtPlot3D library and include files
#  QWTPLOT3D_INCLUDE_DIR, where to find GL/glut.h, etc.
#  QWTPLOT3D_LIBRARIES, the libraries to link against
#  QWTPLOT3D_FOUND, If false, do not try to use GLUT.

find_path( QT_QWTPLOT3D_INCLUDE_DIR qwt3d_plot.h /usr/include/ /usr/local/include /usr/include/qwtplot3d/ /usr/include/qwtplot3d-qt4/ /usr/local/include/qwtplot3d-qt4 )

#message( STATUS "${QWTPLOT3D_INCLUDE_DIR}")
find_library( QWTPLOT3D_LIBRARIES NAMES qwtplot3d-qt4 qwtplot3d /usr/lib/ usr/local/lib )
#message( STATUS "${QWTPLOT3D_LIBRARIES}")
if ( QT_QWTPLOT3D_INCLUDE_DIR  AND QWTPLOT3D_LIBRARIES )
  set( QWTPLOT3D_FOUND TRUE )
  set( QT_QWTPLOT3D_LIBRARY QWTPLOT3D_LIBRARIES )
  set( QT_QWTPLOT3D_LIBRARY_DIRS QWTPLOT3D_LIBRARIES)
endif ( QT_QWTPLOT3D_INCLUDE_DIR  AND QWTPLOT3D_LIBRARIES )

if ( QWTPLOT3D_FOUND )
  if ( NOT QWTPLOT3D_FIND_QUIETLY )
    message( STATUS "Found QwtPlot3D-qt4: ${QWTPLOT3D_LIBRARIES}")
  endif( NOT QWTPLOT3D_FIND_QUIETLY )
else( QWTPLOT3D_FOUND )
  if ( QWTPLOT3D_FIND_REQUIRED )
    message(FATAL_ERROR "Could not find QwtPlot3D-qt4 ")
  endif ( QWTPLOT3D_FIND_REQUIRED )
endif ( QWTPLOT3D_FOUND )
