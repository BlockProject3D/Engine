include("${BP_FRAMEWORK_ROOT}/CMakes/Framework.cmake")

list(APPEND BP_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../")

macro(bp_setup_driver name)
    if (NOT BP_ADDITIONAL_SOURCE_FILE)
        set(BP_ADDITIONAL_SOURCE_FILE "")
    endif(NOT BP_ADDITIONAL_SOURCE_FILE)

    add_library(${name} SHARED ${SOURCES} ${BP_ADDITIONAL_SOURCE_FILE})

    # Attempt at fixing templates problems under MSVC 2017
    target_compile_definitions(${name} PRIVATE "BP_TPL_API=${BP_SYMBOL_EXPORT_MACRO}")

    bp_add_module(${name} "BPF")
	bp_add_module(${PROJECT_NAME} BP3D)

    bp_setup_target(${name} include ${SOURCES})
endmacro(bp_setup_driver)
