include_directories(${FELDRAND_INCLUDE_DIR})

add_library(feldrand SHARED
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
