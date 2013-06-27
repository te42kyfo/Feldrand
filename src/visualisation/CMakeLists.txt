include_directories(${FELDRAND_INCLUDE_DIR}/core) #for grid.hpp
include_directories(${FELDRAND_INCLUDE_DIR}/visualisation)

find_package(OpenGL REQUIRED)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

add_library(feldrand_visualisation SHARED
  DrawArrows.cpp
  DrawArrowsImplementation.cpp
  DrawingRoutine.cpp
  DrawingRoutineImplementation.cpp
  DrawLIC.cpp
  DrawLICImplementation.cpp
  DrawPlain.cpp
  DrawPlainImplementation.cpp
  DrawStreamlines.cpp
  DrawStreamlinesImplementation.cpp
)

set_target_properties(
  feldrand_visualisation
  PROPERTIES
  VERSION ${FELDRAND_VERSION_MAJOR}.${FELDRAND_VERSION_MINOR}
  SOVERSION ${FELDRAND_VISUALISATION_SOVERSION})

target_link_libraries(
  feldrand_visualisation
  ${QT_LIBRARIES}
  ${OPENGL_LIBRARIES})
