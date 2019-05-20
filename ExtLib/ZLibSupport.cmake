# -------------------------------------------------------------
# This function adds the necessary cmake code to find the zlib
# shared libraries and setup custom copy commands and/or install
# rules for Linux and Windows to use
function(AddZLibCopyInstallRules)
  set(options )
  set(oneValueArgs )
  set(multiValueArgs )
  cmake_parse_arguments(zlib "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  set(INTER_DIR ".")


  if(MSVC_IDE)
    set(zlib_TYPES Debug Release)
  else()
    set(zlib_TYPES "${CMAKE_BUILD_TYPE}")
    if("${zlib_TYPES}" STREQUAL "")
        set(zlib_TYPES "Debug")
    endif()
  endif()

  if(WIN32)
    set(zlib_LIB_TYPE "dll")
  else()
    set(zlib_LIB_TYPE "so")
  endif()

  set(zlib_INSTALL_DIR "lib")
  if(WIN32)
    set(zlib_INSTALL_DIR ".")
  endif()

  set(zlib_LIB_DIR "${ZLIB_DIR}/bin")
  set(zlib_LIBVAR "zlib")
  foreach(BTYPE ${zlib_TYPES} )
    # message(STATUS "  BTYPE: ${BTYPE}")
    string(TOUPPER ${BTYPE} UpperBType)
    if(MSVC_IDE)
      set(INTER_DIR "${BTYPE}")
    endif()

    # Set DLL path
    if(${UpperBType} STREQUAL "DEBUG")
      set(zlib_LIBS "zlibd1")
    else()
      set(zlib_LIBS "zlib1")
    endif()
    set(DllLibPath "${zlib_LIB_DIR}/${zlib_LIBS}.${zlib_LIB_TYPE}")

    # Get the Actual Library Path and create Install and copy rules
    # get_target_property(DllLibPath ${zlib_LIBNAME} IMPORTED_LOCATION_${UpperBType})
    # message(STATUS "  DllLibPath: ${DllLibPath}")
    if(NOT "${DllLibPath}" STREQUAL "LibPath-NOTFOUND")
      # message(STATUS "  Creating Install Rule for ${DllLibPath}")
      if(NOT TARGET ZZ_${zlib_LIBVAR}_DLL_${UpperBType}-Copy)
        add_custom_target(ZZ_${zlib_LIBVAR}_DLL_${UpperBType}-Copy ALL
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DllLibPath}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/
                            # COMMENT "  Copy: ${DllLibPath} To: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/"
                            )
        set_target_properties(ZZ_${zlib_LIBVAR}_DLL_${UpperBType}-Copy PROPERTIES FOLDER ZZ_COPY_FILES/${BTYPE}/zlib)
        install(FILES ${DllLibPath} DESTINATION "${zlib_INSTALL_DIR}" CONFIGURATIONS ${BTYPE} COMPONENT Applications)
        get_property(COPY_LIBRARY_TARGETS GLOBAL PROPERTY COPY_LIBRARY_TARGETS)
        set_property(GLOBAL PROPERTY COPY_LIBRARY_TARGETS ${COPY_LIBRARY_TARGETS} ZZ_${zlib_LIBVAR}_DLL_${UpperBType}-Copy)
      endif()
    endif()

    #----------------------------------------------------------------------
    # This section for Linux only
    # message(STATUS "  ${zlib_LIBVAR}_${UpperBType}: ${${zlib_LIBVAR}_${UpperBType}}")
    if(NOT "${${zlib_LIBVAR}_${UpperBType}}" STREQUAL "${zlib_LIBVAR}_${UpperBType}-NOTFOUND" AND NOT WIN32)
      set(SYMLINK_PATH "${${zlib_LIBVAR}_DIR}/${${zlib_LIBVAR}_${UpperBType}}")
      # message(STATUS "  Creating Install Rule for ${SYMLINK_PATH}")
      if(NOT TARGET ZZ_${zlib_LIBVAR}_SYMLINK_${UpperBType}-Copy)
        add_custom_target(ZZ_${zlib_LIBVAR}_SYMLINK_${UpperBType}-Copy ALL
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SYMLINK_PATH}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/
                            # COMMENT "  Copy: ${SYMLINK_PATH} To: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/"
                            )
        set_target_properties(ZZ_${zlib_LIBVAR}_SYMLINK_${UpperBType}-Copy PROPERTIES FOLDER ZZ_COPY_FILES/${BTYPE}/zlib)
        install(FILES ${SYMLINK_PATH} DESTINATION "${zlib_INSTALL_DIR}" CONFIGURATIONS ${BTYPE} COMPONENT Applications)
        get_property(COPY_LIBRARY_TARGETS GLOBAL PROPERTY COPY_LIBRARY_TARGETS)
        set_property(GLOBAL PROPERTY COPY_LIBRARY_TARGETS ${COPY_LIBRARY_TARGETS} ZZ_${zlib_LIBVAR}_SYMLINK_${UpperBType}-Copy) 
      endif()
    endif()
    # End Linux Only Section
    #------------------------------------------------------------------------
  endforeach()
endfunction()

# --------------------------------------------------------------------
# 
function(CMP_FindZLib)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs)
  cmake_parse_arguments(Z "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  # --------------------------------------------------------------------
  # If we are NOT on Apple platform then create the copy and install rules
  # for all of the dependant zlib libraries.
  if(NOT APPLE)
    AddZLibCopyInstallRules()
  endif()
endfunction()
