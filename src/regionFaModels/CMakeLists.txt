set(_FILES
  regionFaModel/regionFaModel.C
  thermalShellModel/thermalShellModel.C
  thermalShellModel/thermalShellModelNew.C
  vibrationShellModel/vibrationShellModel.C
  vibrationShellModel/vibrationShellModelNew.C
  thermalShell/thermalShell.C
  KirchhoffShell/KirchhoffShell.C
  derivedFvPatchFields/thermalShell/thermalShellFvPatchScalarField.C
  derivedFvPatchFields/vibrationShell/vibrationShellFvPatchScalarField.C
  liquidFilm/subModels/kinematic/filmTurbulenceModel/filmTurbulenceModel/filmTurbulenceModel.C
  liquidFilm/subModels/kinematic/filmTurbulenceModel/filmTurbulenceModel/filmTurbulenceModelNew.C
  liquidFilm/subModels/kinematic/filmTurbulenceModel/laminar/laminar.C
  liquidFilm/subModels/kinematic/injectionModel/injectionModelList/injectionModelList.C
  liquidFilm/subModels/kinematic/injectionModel/injectionModel/injectionModel.C
  liquidFilm/subModels/kinematic/injectionModel/injectionModel/injectionModelNew.C
  liquidFilm/subModels/kinematic/injectionModel/curvatureSeparation/curvatureSeparation.C
  liquidFilm/subModels/kinematic/injectionModel/BrunDrippingInjection/BrunDrippingInjection.C
  liquidFilm/subModels/kinematic/force/forceList/forceList.C
  liquidFilm/subModels/kinematic/force/force/force.C
  liquidFilm/subModels/kinematic/force/force/forceNew.C
  liquidFilm/subModels/kinematic/force/contactAngleForces/contactAngleForce/contactAngleForce.C
  liquidFilm/subModels/kinematic/force/contactAngleForces/dynamicContactAngleForce/dynamicContactAngleForce.C
  liquidFilm/subModels/filmSubModelBase.C
  liquidFilm/liquidFilmBase.C
  liquidFilm/liquidFilmBaseNew.C
  liquidFilm/liquidFilmModel/liquidFilmModel.C
  liquidFilm/kinematicThinFilm/kinematicThinFilm.C
  derivedFvPatchFields/filmShell/velocityFilmShellFvPatchVectorField.C
  functionObjects/setTimeStep/setTimeStepFaRegionsFunctionObject.C
)
add_library(regionFaModels ${_FILES})
target_compile_features(regionFaModels PUBLIC cxx_std_11)
set_property(TARGET regionFaModels PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(regionFaModels PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(regionFaModels PUBLIC distributionModels faOptions)
target_include_directories(regionFaModels PUBLIC
  .
)
install(TARGETS regionFaModels DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
