set(_FILES
  thermalBaffleModel/thermalBaffleModel.C
  thermalBaffleModel/thermalBaffleModelNew.C
  thermalBaffle/thermalBaffle.C
  noThermo/noThermo.C
  derivedFvPatchFields/thermalBaffle/thermalBaffleFvPatchScalarField.C
)
add_library(thermalBaffleModels ${_FILES})
target_compile_features(thermalBaffleModels PUBLIC cxx_std_11)
set_property(TARGET thermalBaffleModels PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(thermalBaffleModels PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(thermalBaffleModels PUBLIC dynamicMesh radiationModels regionModels thermoTools)
target_include_directories(thermalBaffleModels PUBLIC
  .
)
install(TARGETS thermalBaffleModels DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
