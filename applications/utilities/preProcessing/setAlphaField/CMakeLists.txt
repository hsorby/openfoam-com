set(_FILES
  setAlphaField.C
)
add_executable(setAlphaField ${_FILES})
target_compile_features(setAlphaField PUBLIC cxx_std_11)
target_link_libraries(setAlphaField PUBLIC alphaFieldFunctions geometricVoF twoPhaseProperties)
target_include_directories(setAlphaField PUBLIC
  .
)
