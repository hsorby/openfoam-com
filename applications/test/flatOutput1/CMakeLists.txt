set(_FILES
  Test-flatOuput1.C
)
add_executable(Test-flatOuput1 ${_FILES})
target_compile_features(Test-flatOuput1 PUBLIC cxx_std_11)
target_include_directories(Test-flatOuput1 PUBLIC
  .
)
