set(_FILES
  foamCalc.C
)
add_executable(foamCalc ${_FILES})
target_compile_features(foamCalc PUBLIC cxx_std_11)
target_include_directories(foamCalc PUBLIC
  .
)
