set(_FILES
  compressibleTwoPhaseMixtureTurbulenceModels.C
)
add_library(compressibleTwoPhaseMixtureTurbulenceModels ${_FILES})
target_compile_features(compressibleTwoPhaseMixtureTurbulenceModels PUBLIC cxx_std_11)
set_property(TARGET compressibleTwoPhaseMixtureTurbulenceModels PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(compressibleTwoPhaseMixtureTurbulenceModels PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(compressibleTwoPhaseMixtureTurbulenceModels PUBLIC immiscibleIncompressibleTwoPhaseMixture incompressibleTurbulenceModels thermoTools)
target_include_directories(compressibleTwoPhaseMixtureTurbulenceModels PUBLIC
  .
)
install(TARGETS compressibleTwoPhaseMixtureTurbulenceModels DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
