set(_FILES
  zoltanRenumber.C
)
add_library(zoltanRenumber ${_FILES})
target_compile_features(zoltanRenumber PUBLIC cxx_std_11)
set_property(TARGET zoltanRenumber PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(zoltanRenumber PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(zoltanRenumber PUBLIC meshTools)
target_include_directories(zoltanRenumber PUBLIC
  .
)
install(TARGETS zoltanRenumber DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
