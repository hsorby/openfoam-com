set(_FILES
  Test-field1.C
)
add_executable(Test-field1 ${_FILES})
target_compile_features(Test-field1 PUBLIC cxx_std_11)
target_include_directories(Test-field1 PUBLIC
  .
)
