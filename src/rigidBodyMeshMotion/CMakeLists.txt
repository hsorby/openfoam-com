set(_FILES
  rigidBodyMeshMotion/rigidBodyMeshMotion.C
  rigidBodyMeshMotionSolver/rigidBodyMeshMotionSolver.C
)
add_library(rigidBodyMeshMotion ${_FILES})
target_compile_features(rigidBodyMeshMotion PUBLIC cxx_std_11)
set_property(TARGET rigidBodyMeshMotion PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(rigidBodyMeshMotion PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(rigidBodyMeshMotion PUBLIC dynamicMesh forces rigidBodyDynamics)
target_include_directories(rigidBodyMeshMotion PUBLIC
  .
)
install(TARGETS rigidBodyMeshMotion DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
