set(_FILES
  patchSummary.C
)
add_executable(patchSummary ${_FILES})
target_compile_features(patchSummary PUBLIC cxx_std_11)
target_link_libraries(patchSummary PUBLIC genericPatchFields)
target_include_directories(patchSummary PUBLIC
  .
)
