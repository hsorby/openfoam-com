set(_FILES
  regionModel/regionModel.C
  singleLayerRegion/singleLayerRegion.C
  regionModel1D/regionModel1D.C
  derivedFvPatches/mappedVariableThicknessWall/mappedVariableThicknessWallFvPatch.C
  regionModelFunctionObject/regionModelFunctionObject/regionModelFunctionObject.C
  regionModelFunctionObject/regionModelFunctionObject/regionModelFunctionObjectNew.C
  regionModelFunctionObject/regionModelFunctionObject/regionModelFunctionObjectList.C
)
add_library(regionModels ${_FILES})
target_compile_features(regionModels PUBLIC cxx_std_11)
set_property(TARGET regionModels PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(regionModels PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(regionModels PUBLIC finiteVolume meshTools)
target_include_directories(regionModels PUBLIC
  .
)
install(TARGETS regionModels DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
