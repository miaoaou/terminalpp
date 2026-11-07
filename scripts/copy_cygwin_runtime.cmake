if(NOT DEFINED SOURCE_DIR OR NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "SOURCE_DIR and DEST_DIR must be provided")
endif()

if(NOT EXISTS "${SOURCE_DIR}/bin/cygwin1.dll")
    message(FATAL_ERROR "The bundled Cygwin runtime is missing: ${SOURCE_DIR}")
endif()

find_program(POWERSHELL_EXECUTABLE NAMES pwsh powershell)
if(NOT POWERSHELL_EXECUTABLE)
    message(FATAL_ERROR "PowerShell is required to prepare the bundled Cygwin runtime")
endif()

execute_process(
    COMMAND "${POWERSHELL_EXECUTABLE}"
        -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass
        -File "${CMAKE_CURRENT_LIST_DIR}/prepare_cygwin_runtime.ps1"
        -SourceRoot "${SOURCE_DIR}"
        -DestinationRoot "${DEST_DIR}"
    RESULT_VARIABLE prepare_result
    OUTPUT_VARIABLE prepare_output
    ERROR_VARIABLE prepare_error
)

if(NOT prepare_result EQUAL 0)
    message(FATAL_ERROR "Preparing the bundled Cygwin runtime failed: ${prepare_error}")
endif()

if(NOT prepare_output STREQUAL "")
    string(STRIP "${prepare_output}" prepare_output)
    message(STATUS "${prepare_output}")
endif()
