set(_FILES
  SRFPimpleFoam.C
)
add_executable(SRFPimpleFoam ${_FILES})
target_compile_features(SRFPimpleFoam PUBLIC cxx_std_11)
target_link_libraries(SRFPimpleFoam PUBLIC fvOptions incompressibleTransportModels incompressibleTurbulenceModels sampling)
target_include_directories(SRFPimpleFoam PUBLIC
  .
)
