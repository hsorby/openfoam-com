set(_FILES
  implicitFunctions/implicitFunction.C
  implicitFunctions/sphere/sphereImplicitFunction.C
  implicitFunctions/sin/sinImplicitFunction.C
  implicitFunctions/ellipsoid/ellipsoidImplicitFunction.C
  implicitFunctions/paraboloid/paraboloidImplicitFunction.C
  implicitFunctions/plane/planeImplicitFunction.C
  implicitFunctions/cylinder/cylinderImplicitFunction.C
  implicitFunctions/composedFunction/composedFunctionImplicitFunction.C
)
add_library(alphaFieldFunctions ${_FILES})
target_compile_features(alphaFieldFunctions PUBLIC cxx_std_11)
set_property(TARGET alphaFieldFunctions PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(alphaFieldFunctions PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_include_directories(alphaFieldFunctions PUBLIC
  .
)
install(TARGETS alphaFieldFunctions DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
