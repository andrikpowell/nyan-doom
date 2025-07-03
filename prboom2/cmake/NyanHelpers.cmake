include_guard()

function(nyan_fail_if_invalid_target tgt)
  if(NOT TARGET ${tgt})
    message(FATAL_ERROR "${tgt} is not a valid CMake target.")
  endif()
endfunction()
