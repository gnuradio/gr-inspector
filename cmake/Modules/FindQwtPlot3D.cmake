# - try to find QwtPlot3D library and include files
#  QWTPLOT3D_INCLUDE_DIR, where to find GL/glut.h, etc.
#  QWTPLOT3D_LIBRARIES, the libraries to link against
#  QWTPLOT3D_FOUND, If false, do not try to use GLUT.

FIND_PATH( QT_QWTPLOT3D_INCLUDE_DIR qwt3d_plot.h /usr/include/ /usr/local/include /usr/include/qwtplot3d/ /usr/include/qwtplot3d-qt4/ /usr/local/include/qwtplot3d-qt4 )

#MESSAGE( STATUS "${QWTPLOT3D_INCLUDE_DIR}")
FIND_LIBRARY( QT_QWTPLOT3D_LIBRARY qwtplot3d qwtplot3d-qt4 /usr/lib/ usr/local/lib ) 
#MESSAGE( STATUS "${QWTPLOT3D_LIBRARIES}")
IF ( QT_QWTPLOT3D_INCLUDE_DIR  AND QWTPLOT3D_LIBRARIES ) 
  SET( QWTPLOT3D_FOUND TRUE ) 
ENDIF ( QWTPLOT3D_INCLUDE_DIR  AND QWTPLOT3D_LIBRARIES ) 

IF ( QWTPLOT3D_FOUND ) 
  IF ( NOT QWTPLOT3D_FIND_QUIETLY ) 
    MESSAGE( STATUS "Found QwtPlot3D-qt4: ${QWTPLOT3D_LIBRARIES}")
  ENDIF( NOT QWTPLOT3D_FIND_QUIETLY ) 
ELSE( QWTPLOT3D_FOUND ) 
  IF ( QWTPLOT3D_FIND_REQUIRED ) 
    MESSAGE(FATAL_ERROR "Could not find QwtPlot3D-qt4 ") 
  ENDIF ( QWTPLOT3D_FIND_REQUIRED ) 
ENDIF ( QWTPLOT3D_FOUND ) 

