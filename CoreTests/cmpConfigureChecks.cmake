
# In this file we are doing all of our 'configure' checks. Modern C++ systems that
# implement the C++11/14/17 standard all now make guarantees about sizes of types
# that are used.

INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)


# Do a very simple check for the system processor. We really only support compiling
# on a Little Endian system
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
  set(BIGENDIAN 0)
else()
  set(BIGENDIAN 1)
endif()

# We also assume a 64 bit system for compiling.

set(CMP_HAVE_STDINT_H 1)

set(CMP_SIZEOF_CHAR 1)
set(CMP_TYPE_CHAR_IS_SIGNED 1)

set(CMP_SIZEOF_SHORT 2)
set(CMP_SIZEOF_INT 4)
set(CMP_SIZEOF_DOUBLE 8)
set(CMP_SIZEOF_FLOAT 4)
set(CMP_SIZEOF_INT8_T 1)
set(CMP_SIZEOF_INT16_T 2)
set(CMP_SIZEOF_INT32_T 4)
set(CMP_SIZEOF_INT64_T 8)


set(CMP_SIZEOF_OFF64_T 8)
set(CMP_SIZEOF_OFF_T 8)
set(CMP_SIZEOF_UINT16_T 2)
set(CMP_SIZEOF_UINT32_T 4)
set(CMP_SIZEOF_UINT64_T 8)
set(CMP_SIZEOF_UINT8_T 1)


# This assumes a 64 bit system
set(CMP_SIZEOF_SIZE_T 8)
set(CMP_SIZEOF_SSIZE_T 8)


#-----------------------------------------------------------------------------
#  Check the size in bytes of all the int and float types
#-----------------------------------------------------------------------------
MACRO ( CORE_CHECK_TYPE_SIZE type var)
    set(aType ${type})
    set(aVar  ${var})
    CHECK_TYPE_SIZE(${aType}   ${aVar})
    if(NOT ${aVar})
        set(${aVar} 0 CACHE INTERNAL "SizeOf for ${aType}")
    endif()
ENDMACRO()

set(CMP_SIZEOF_CHAR 1)
set(CMP_SIZEOF_SHORT 2)
set(CMP_SIZEOF_INT 4)
set(CMP_SIZEOF_UNSIGNED 4)

if(NOT APPLE)
    CORE_CHECK_TYPE_SIZE(long           CMP_SIZEOF_LONG)
    CORE_CHECK_TYPE_SIZE("long long"    CMP_SIZEOF_LONG_LONG)
endif()

CORE_CHECK_TYPE_SIZE(__int64        CMP_SIZEOF___INT64)
if(NOT CMP_SIZEOF___INT64)
    set(CMP_SIZEOF___INT64 0)
endif()
CORE_CHECK_TYPE_SIZE("long double"  CMP_SIZEOF_LONG_DOUBLE)

#-----------------------------------------------------------------------------
#  Check the size in bytes of all the int and float types
#-----------------------------------------------------------------------------
MACRO ( CORE_CHECK_TYPE_SIZE_ALT type var size)
  set(aType ${type})
  set(aVar  ${var})
  set(${aVar} ${size} CACHE INTERNAL "SizeOf for ${aType}")
  #message(STATUS "C++11 <cstdint> SizeOf for ${aType} = ${size}")
ENDMACRO()

if(NOT APPLE)
    CORE_CHECK_TYPE_SIZE(size_t         CMP_SIZEOF_SIZE_T)
    CORE_CHECK_TYPE_SIZE(ssize_t        CMP_SIZEOF_SSIZE_T)
    if(NOT CMP_SIZEOF_SSIZE_T)
        set(CMP_SIZEOF_SSIZE_T 0)
    endif()
endif()
CORE_CHECK_TYPE_SIZE(off_t          CMP_SIZEOF_OFF_T)
CORE_CHECK_TYPE_SIZE(off64_t        CMP_SIZEOF_OFF64_T)
if(NOT CMP_SIZEOF_OFF64_T)
    set(CMP_SIZEOF_OFF64_T 0)
endif()


