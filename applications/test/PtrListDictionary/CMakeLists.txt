set(_FILES
  Test-PtrListDictionary.C
)
add_executable(Test-PtrListDictionary ${_FILES})
target_compile_features(Test-PtrListDictionary PUBLIC cxx_std_11)
target_include_directories(Test-PtrListDictionary PUBLIC
  .
)
