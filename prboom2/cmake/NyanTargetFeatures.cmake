include_guard()

include(NyanHelpers)

# Internal functions, these should not be called from outside this module

function(nyan_internal_setup_warnings_msvc result_var)
  set(${result_var}
    "/W3"
    PARENT_SCOPE
  )
endfunction()

function(nyan_internal_setup_warnings_gnu result_var)
  set(GNU_WARNINGS
    "-Wall"
    "-Wwrite-strings"
    "-Wundef"
    "-Wtype-limits"
    "-Wcast-qual"
    "-Wpointer-arith"
    "-Wno-unused-function"
    "-Wno-switch"
    "-Wno-sign-compare"
    "-Wno-format-truncation"
    "-Wno-missing-field-initializers"
  )
  set(GNU_C_WARNINGS
    -Wabsolute-value
    -Wno-pointer-sign
    -Wdeclaration-after-statement
    -Wbad-function-cast
  )
  set(GNU_WARNINGS_SET ${GNU_WARNINGS} ${GNU_C_WARNINGS})

  include(CheckCCompilerFlag)
  check_c_compiler_flag("${GNU_WARNINGS_SET}" NYAN_SUPPORTS_GNU_WARNINGS)

  if(NOT NYAN_SUPPORTS_GNU_WARNINGS)
    message(AUTHOR_WARNING
      "The default set of warnings is not supported.\n"
      "Set 'NYAN_ENABLED_WARNINGS' manually to a list of warnings you wish to enable."
    )
    set(SUPPORTED_GNU_WARNINGS "")
  else()
    set(SUPPORTED_GNU_WARNINGS ${GNU_WARNINGS})
    foreach(c_warning IN LISTS GNU_C_WARNINGS)
      list(APPEND
        SUPPORTED_GNU_WARNINGS
        $<$<COMPILE_LANGUAGE:C>:${c_warning}>
      )
    endforeach()
  endif()

  set(${result_var}
    "${SUPPORTED_GNU_WARNINGS}"
    PARENT_SCOPE
  )
endfunction()

function(nyan_internal_setup_warnings)
  if(MSVC)
    nyan_internal_setup_warnings_msvc(PLATFORM_WARNINGS)
  else()
    nyan_internal_setup_warnings_gnu(PLATFORM_WARNINGS)
  endif()
  set(NYAN_ENABLED_WARNINGS
    "${PLATFORM_WARNINGS}"
    CACHE STRING
    "List of compiler warnings"
  )
endfunction()

function(nyan_internal_check_fast_math_flag)
  if(MSVC)
    set(FAST_MATH_FLAG "/fp:fast")
  else()
    set(FAST_MATH_FLAG "-ffast-math")
  endif()
  include(CheckCCompilerFlag)
  check_c_compiler_flag("${FAST_MATH_FLAG}" NYAN_SUPPORTS_FAST_MATH)

  if(NOT NYAN_SUPPORTS_FAST_MATH)
    message(AUTHOR_WARNING
      "Could not detect the flag to enable fast math.\n"
      "Set 'NYAN_FAST_MATH_FLAG' manually to a list of flags needed to enable it."
    )
    set(FAST_MATH_FLAG "")
  endif()

  set(NYAN_FAST_MATH_FLAG
    "${FAST_MATH_FLAG}"
    CACHE STRING
    "Flags to enable fast math"
  )
endfunction()

# Public functions

function(nyan_target_set_warnings tgt)
  nyan_fail_if_invalid_target(${tgt})
  
  if(NOT DEFINED CACHE{NYAN_ENABLED_WARNINGS})
    nyan_internal_setup_warnings()
  endif()

  target_compile_options(${tgt}
    PRIVATE
    ${NYAN_ENABLED_WARNINGS}
  )
endfunction()

function(nyan_target_silence_deprecation tgt)
  nyan_fail_if_invalid_target(${tgt})

  if(WIN32)
    target_compile_definitions(${tgt}
      PRIVATE
      "_CRT_SECURE_NO_WARNINGS" # Warnings for not using "secure" (_s) functions
      "_CRT_NONSTDC_NO_WARNINGS" # Warnings for using POSIX names of functions
    )
  elseif(APPLE)
    target_compile_definitions(${tgt}
      PRIVATE
      "GL_SILENCE_DEPRECATION" # Calling GL functions is deprecated on macOS
    )
  endif()
endfunction()

function(nyan_target_enable_fast_math tgt)
  nyan_fail_if_invalid_target(${tgt})

  if(NOT DEFINED CACHE{NYAN_FAST_MATH_FLAG})
    nyan_internal_check_fast_math_flag()
  endif()

  target_compile_options(${tgt}
    PRIVATE
    ${NYAN_FAST_MATH_FLAG}
  )
endfunction()
