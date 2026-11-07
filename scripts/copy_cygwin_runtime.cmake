if(NOT DEFINED SOURCE_DIR OR NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "SOURCE_DIR and DEST_DIR must be provided")
endif()

if(NOT EXISTS "${SOURCE_DIR}/bin/cygwin1.dll")
    message(FATAL_ERROR "The bundled Cygwin runtime is missing: ${SOURCE_DIR}")
endif()

set(runtime_stamp "${DEST_DIR}/.terminalpp-cygwin-runtime")
# The runtime is large; avoid copying it again on every relink. Remove the
# output stamp when refreshing the bundled Cygwin files.
if(EXISTS "${runtime_stamp}" AND EXISTS "${DEST_DIR}/bin/cygwin1.dll")
    return()
endif()

file(MAKE_DIRECTORY "${DEST_DIR}")
file(COPY "${SOURCE_DIR}/" DESTINATION "${DEST_DIR}")
file(MAKE_DIRECTORY "${DEST_DIR}/dev" "${DEST_DIR}/home" "${DEST_DIR}/tmp")
file(WRITE "${runtime_stamp}" "Bundled Cygwin runtime copied by terminalpp CMake build.\n")
