set(_FILES
  Test-HashPtrTable.C
)
add_executable(Test-HashPtrTable ${_FILES})
target_compile_features(Test-HashPtrTable PUBLIC cxx_std_11)
target_include_directories(Test-HashPtrTable PUBLIC
  .
)
