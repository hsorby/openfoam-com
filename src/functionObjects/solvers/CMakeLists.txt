set(_FILES
  scalarTransport/scalarTransport.C
  energyTransport/energyTransport.C
  electricPotential/electricPotential.C
)
add_library(solverFunctionObjects ${_FILES})
target_compile_features(solverFunctionObjects PUBLIC cxx_std_11)
set_property(TARGET solverFunctionObjects PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(solverFunctionObjects PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(solverFunctionObjects PUBLIC compressibleTurbulenceModels fvOptions incompressibleTurbulenceModels specie)
target_include_directories(solverFunctionObjects PUBLIC
  .
)
install(TARGETS solverFunctionObjects DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
