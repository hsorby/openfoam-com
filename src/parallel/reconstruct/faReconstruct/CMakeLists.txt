set(_FILES
  processorFaMeshes.C
  faFieldReconstructor.C
  faMeshReconstructor.C
)
add_library(faReconstruct ${_FILES})
target_compile_features(faReconstruct PUBLIC cxx_std_11)
set_property(TARGET faReconstruct PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(faReconstruct PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(faReconstruct PUBLIC finiteArea finiteVolume meshTools)
target_include_directories(faReconstruct PUBLIC
  .
)
install(TARGETS faReconstruct DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
