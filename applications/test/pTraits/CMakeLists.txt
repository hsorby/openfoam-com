set(_FILES
  Test-pTraits.C
)
add_executable(Test-pTraits ${_FILES})
target_compile_features(Test-pTraits PUBLIC cxx_std_11)
target_include_directories(Test-pTraits PUBLIC
  .
)
