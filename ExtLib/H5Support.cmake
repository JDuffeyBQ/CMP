# Adds install rules for H5Support
function(AddH5SupportInstallRules)
  set(options)
  set(oneValueArgs TARGET)
  set(multiValueArgs TYPES)
  cmake_parse_arguments(Z "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(INSTALL_DIR "lib")
  if(WIN32)
    set(INSTALL_DIR ".")
  endif()

  foreach(TYPE ${Z_TYPES})
    string(TOUPPER ${TYPE} TYPE_UPPER)
    get_target_property(LIB_PATH ${Z_TARGET} IMPORTED_LOCATION_${TYPE_UPPER})
    if(NOT "${LIB_PATH}" STREQUAL "LIB_PATH-NOTFOUND")
      install(FILES ${LIB_PATH} DESTINATION ${INSTALL_DIR} CONFIGURATIONS ${TYPE} COMPONENT Applications)
    endif()  
  endforeach()
endfunction()

# Adds copy custom target for H5Support
function(AddH5SupportCopyRules)
  set(options)
  set(oneValueArgs NAME TARGET)
  set(multiValueArgs TYPES)
  cmake_parse_arguments(Z "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(INTER_DIR ".")

  foreach(TYPE ${Z_TYPES})
    string(TOUPPER ${TYPE} TYPE_UPPER)
    if(MSVC_IDE)
      set(INTER_DIR "${TYPE}")
    endif()
    get_target_property(LIB_PATH ${Z_TARGET} IMPORTED_LOCATION_${TYPE_UPPER})
    if(NOT "${LIB_PATH}" STREQUAL "LIB_PATH-NOTFOUND")
      if(NOT TARGET ZZ_${Z_NAME}_DLL_${TYPE}-Copy)
        add_custom_target(ZZ_${Z_NAME}_DLL_${TYPE}-Copy ALL
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIB_PATH}
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/
        )
        set_target_properties(ZZ_${Z_NAME}_DLL_${TYPE}-Copy PROPERTIES FOLDER ZZ_COPY_FILES/${TYPE}/${Z_NAME})
      endif()
    endif()
  endforeach()
endfunction()

# Adds copy targets and/or install rules for H5Support. The target H5Support::H5Support must exist.
function(CMP_FindH5Support)
  set(options COPY INSTALL)
  set(oneValueArgs)
  set(multiValueArgs)
  cmake_parse_arguments(Z "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT APPLE)
    if(MSVC_IDE)
      set(BUILD_TYPES Debug Release)
    else()
      set(BUILD_TYPES "${CMAKE_BUILD_TYPE}")
      if("${BUILD_TYPES}" STREQUAL "")
          set(BUILD_TYPES "Debug")
      endif()
    endif()

    set(H5Support_TARGET H5Support::H5Support)

    if(NOT TARGET ${H5Support_TARGET})
      message(FATAL_ERROR "Target \"${H5Support_TARGET}\" does not exist")
    endif()

    if(Z_COPY)
      AddH5SupportCopyRules(NAME "H5Support" TARGET ${H5Support_TARGET} TYPES ${BUILD_TYPES})
    endif()

    if(Z_INSTALL)
      AddH5SupportInstallRules(TARGET ${H5Support_TARGET} TYPES ${BUILD_TYPES})
    endif()
  endif()
endfunction()
