# -------------------------------------------------------------
# This function adds the necessary cmake code to find the libarchive
# shared libraries and setup custom copy commands and/or install
# rules for Linux and Windows to use
function(AddLibArchiveCopyInstallRules)
  set(options )
  set(oneValueArgs )
  set(multiValueArgs )
  cmake_parse_arguments(libarchive "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  set(INTER_DIR ".")


  if(MSVC_IDE)
    set(libarchive_TYPES Debug Release)
  else()
    set(libarchive_TYPES "${CMAKE_BUILD_TYPE}")
    if("${libarchive_TYPES}" STREQUAL "")
        set(libarchive_TYPES "Debug")
    endif()
  endif()

  if(WIN32)
    set(libarchive_LIB_TYPE "dll")
  else()
    set(libarchive_LIB_TYPE "so")
  endif()

  set(libarchive_INSTALL_DIR "lib")
  if(WIN32)
    set(libarchive_INSTALL_DIR ".")
  endif()

  set(libarchive_LIB_DIR "${LIBARCHIVE_DIR}/bin")
  set(libarchive_LIBVAR "libarchive")
  foreach(BTYPE ${libarchive_TYPES} )
    # message(STATUS "  BTYPE: ${BTYPE}")
    string(TOUPPER ${BTYPE} UpperBType)
    if(MSVC_IDE)
      set(INTER_DIR "${BTYPE}")
    endif()

    # Set DLL path
    if(${UpperBType} STREQUAL "DEBUG")
      set(libarchive_LIBS "archive_d")
    else()
      set(libarchive_LIBS "archive")
    endif()
    set(DllLibPath "${libarchive_LIB_DIR}/${libarchive_LIBS}.${libarchive_LIB_TYPE}")

    # Get the Actual Library Path and create Install and copy rules
    # get_target_property(DllLibPath ${libarchive_LIBNAME} IMPORTED_LOCATION_${UpperBType})
    # message(STATUS "  DllLibPath: ${DllLibPath}")
    if(NOT "${DllLibPath}" STREQUAL "LibPath-NOTFOUND")
      # message(STATUS "  Creating Install Rule for ${DllLibPath}")
      if(NOT TARGET ZZ_${libarchive_LIBVAR}_DLL_${UpperBType}-Copy)
        add_custom_target(ZZ_${libarchive_LIBVAR}_DLL_${UpperBType}-Copy ALL
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DllLibPath}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/
                            # COMMENT "  Copy: ${DllLibPath} To: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/"
                            )
        set_target_properties(ZZ_${libarchive_LIBVAR}_DLL_${UpperBType}-Copy PROPERTIES FOLDER ZZ_COPY_FILES/${BTYPE}/libarchive)
        install(FILES ${DllLibPath} DESTINATION "${libarchive_INSTALL_DIR}" CONFIGURATIONS ${BTYPE} COMPONENT Applications)
        get_property(COPY_LIBRARY_TARGETS GLOBAL PROPERTY COPY_LIBRARY_TARGETS)
        set_property(GLOBAL PROPERTY COPY_LIBRARY_TARGETS ${COPY_LIBRARY_TARGETS} ZZ_${libarchive_LIBVAR}_DLL_${UpperBType}-Copy)
      endif()
    endif()

    #----------------------------------------------------------------------
    # This section for Linux only
    # message(STATUS "  ${libarchive_LIBVAR}_${UpperBType}: ${${libarchive_LIBVAR}_${UpperBType}}")
    if(NOT "${${libarchive_LIBVAR}_${UpperBType}}" STREQUAL "${libarchive_LIBVAR}_${UpperBType}-NOTFOUND" AND NOT WIN32)
      set(SYMLINK_PATH "${${libarchive_LIBVAR}_DIR}/${${libarchive_LIBVAR}_${UpperBType}}")
      # message(STATUS "  Creating Install Rule for ${SYMLINK_PATH}")
      if(NOT TARGET ZZ_${libarchive_LIBVAR}_SYMLINK_${UpperBType}-Copy)
        add_custom_target(ZZ_${libarchive_LIBVAR}_SYMLINK_${UpperBType}-Copy ALL
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SYMLINK_PATH}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/
                            # COMMENT "  Copy: ${SYMLINK_PATH} To: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INTER_DIR}/"
                            )
        set_target_properties(ZZ_${libarchive_LIBVAR}_SYMLINK_${UpperBType}-Copy PROPERTIES FOLDER ZZ_COPY_FILES/${BTYPE}/libarchive)
        install(FILES ${SYMLINK_PATH} DESTINATION "${libarchive_INSTALL_DIR}" CONFIGURATIONS ${BTYPE} COMPONENT Applications)
        get_property(COPY_LIBRARY_TARGETS GLOBAL PROPERTY COPY_LIBRARY_TARGETS)
        set_property(GLOBAL PROPERTY COPY_LIBRARY_TARGETS ${COPY_LIBRARY_TARGETS} ZZ_${libarchive_LIBVAR}_SYMLINK_${UpperBType}-Copy) 
      endif()
    endif()
    # End Linux Only Section
    #------------------------------------------------------------------------
  endforeach()
endfunction()

# --------------------------------------------------------------------
# 
function(CMP_FindLibArchive)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs)
  cmake_parse_arguments(Z "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  # --------------------------------------------------------------------
  # If we are NOT on Apple platform then create the copy and install rules
  # for all of the dependant libarchive libraries.
  if(NOT APPLE)
    AddLibArchiveCopyInstallRules()
  endif()
endfunction()
