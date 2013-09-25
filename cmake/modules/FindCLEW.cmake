#
# Try to find CLEW library and include path.
# Once done this will define
#
# CLEW_FOUND
# CLEW_LIBRARY
# OPENCLHELPER_LIBRARY
# stub!
#

SET( CLEW_LIBRARY ${FELDRAND_INCLUDE_DIR}/OpenClHelper/libclew.so)
SET( OPENCLHELPER_LIBRARY ${FELDRAND_INCLUDE_DIR}/OpenClHelper/libOpenCLHelper.so)
SET( CLEW_FOUND 1 CACHE STRING "Set to 1 if CLEW is found, 0 otherwise")


MARK_AS_ADVANCED( CLEW_FOUND )
