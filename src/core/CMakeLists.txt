include_directories(${FELDRAND_INCLUDE_DIR})

find_package(CLEW REQUIRED)

add_library(feldrand SHARED
  lodepng.cc
  MRT_LBM.cpp
  BGK_OCL.cpp
  SimulationImplementation.cpp
  Simulation.cpp
  SimulationUtilities.cpp
)



set_target_properties(
  feldrand
  PROPERTIES
  VERSION ${FELDRAND_VERSION_MAJOR}.${FELDRAND_VERSION_MINOR}
  SOVERSION ${FELDRAND_CORE_SOVERSION})

target_link_libraries(
		feldrand
		${CLEW_LIBRARY}
		${OPENCLHELPER_LIBRARY} )
