set(_FILES
  smoothSurfaceData.C
)
add_executable(smoothSurfaceData ${_FILES})
target_compile_features(smoothSurfaceData PUBLIC cxx_std_11)
target_link_libraries(smoothSurfaceData PUBLIC meshTools)
target_include_directories(smoothSurfaceData PUBLIC
  .
)
