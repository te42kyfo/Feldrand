include_directories(${FELDRAND_INCLUDE_DIR}/core)
include_directories(${FELDRAND_INCLUDE_DIR}/visualisation)
include_directories(${FELDRAND_INCLUDE_DIR}/gui)
include_directories(${FELDRAND_INCLUDE_DIR})

find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTMAIN TRUE)

set(feldrand_gui_SRCS
  main.cpp
  MainWindow.cpp
  OpenGLWidget.cpp
  AboutWindow.cpp
#  NewWindow.cpp
)

set(feldrand_gui_MOC_HEADERS
  ${FELDRAND_INCLUDE_DIR}/gui/MainWindow.hpp
  ${FELDRAND_INCLUDE_DIR}/gui/OpenGLWidget.hpp
  ${FELDRAND_INCLUDE_DIR}/gui/AboutWindow.hpp
#  ${FELDRAND_INCLUDE_DIR}/gui/NewWindow.hpp
)
set(feldrand_gui_RESOURCES "feldrand_gui.qrc")

qt4_add_resources(feldrand_gui_RESOURCES_SRCS
  ${feldrand_gui_RESOURCES})

QT4_WRAP_CPP(feldrand_gui_MOC
  ${feldrand_gui_MOC_HEADERS})

include(${QT_USE_FILE})
include_directories(${OPENGL_INCLUDE_DIR})
add_definitions(${QT_DEFINITIONS})
include_directories( ${CMAKE_BINARY_DIR} )

add_executable(feldrand_gui
  ${feldrand_gui_SRCS}
  ${feldrand_gui_MOC}
  ${feldrand_gui_RESOURCES_SRCS}
)

set_target_properties(feldrand_gui PROPERTIES
  OUTPUT_NAME feldrand)

target_link_libraries(feldrand_gui
  feldrand
  feldrand_visualisation
 -lGLEW		
 ${QT_LIBRARIES}		
  ${OPENGL_LIBRARIES}
 
)
