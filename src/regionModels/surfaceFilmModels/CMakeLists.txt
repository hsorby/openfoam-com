set(_FILES
  surfaceFilmModel/surfaceFilmModel.C
  surfaceFilmModel/surfaceFilmModelNew.C
  surfaceFilmRegionModel/surfaceFilmRegionModel.C
  noFilm/noFilm.C
  kinematicSingleLayer/kinematicSingleLayer.C
  thermoSingleLayer/thermoSingleLayer.C
  submodels/filmSubModelBase.C
  submodels/kinematic/force/force/force.C
  submodels/kinematic/force/force/forceNew.C
  submodels/kinematic/force/forceList/forceList.C
  submodels/kinematic/force/contactAngleForces/contactAngleForce/contactAngleForce.C
  submodels/kinematic/force/contactAngleForces/distribution/distributionContactAngleForce.C
  submodels/kinematic/force/contactAngleForces/temperatureDependent/temperatureDependentContactAngleForce.C
  submodels/kinematic/force/contactAngleForces/perturbedTemperatureDependent/perturbedTemperatureDependentContactAngleForce.C
  submodels/kinematic/force/thermocapillaryForce/thermocapillaryForce.C
  submodels/kinematic/injectionModel/injectionModel/injectionModel.C
  submodels/kinematic/injectionModel/injectionModel/injectionModelNew.C
  submodels/kinematic/injectionModel/injectionModelList/injectionModelList.C
  submodels/kinematic/injectionModel/drippingInjection/drippingInjection.C
  submodels/kinematic/injectionModel/BrunDrippingInjection/BrunDrippingInjection.C
  submodels/kinematic/injectionModel/patchInjection/patchInjection.C
  submodels/kinematic/injectionModel/curvatureSeparation/curvatureSeparation.C
  submodels/kinematic/transferModels/transferModel/transferModel.C
  submodels/kinematic/transferModels/transferModel/transferModelNew.C
  submodels/kinematic/transferModels/transferModelList/transferModelList.C
  submodels/kinematic/filmThermoModel/filmThermoModel/filmThermoModel.C
  submodels/kinematic/filmThermoModel/filmThermoModel/filmThermoModelNew.C
  submodels/kinematic/filmThermoModel/constantFilmThermo/constantFilmThermo.C
  submodels/kinematic/filmThermoModel/liquidFilmThermo/liquidFilmThermo.C
  submodels/kinematic/filmTurbulenceModel/filmTurbulenceModel/filmTurbulenceModel.C
  submodels/kinematic/filmTurbulenceModel/filmTurbulenceModel/filmTurbulenceModelNew.C
  submodels/kinematic/filmTurbulenceModel/laminar/laminar.C
  submodels/thermo/phaseChangeModel/phaseChangeModel/phaseChangeModel.C
  submodels/thermo/phaseChangeModel/phaseChangeModel/phaseChangeModelNew.C
  submodels/thermo/phaseChangeModel/noPhaseChange/noPhaseChange.C
  submodels/thermo/phaseChangeModel/standardPhaseChange/standardPhaseChange.C
  submodels/thermo/phaseChangeModel/solidification/solidification.C
  submodels/thermo/phaseChangeModel/waxSolventEvaporation/waxSolventEvaporation.C
  submodels/thermo/heatTransferModel/heatTransferModel/heatTransferModel.C
  submodels/thermo/heatTransferModel/heatTransferModel/heatTransferModelNew.C
  submodels/thermo/heatTransferModel/constantHeatTransfer/constantHeatTransfer.C
  submodels/thermo/heatTransferModel/mappedConvectiveHeatTransfer/mappedConvectiveHeatTransfer.C
  submodels/thermo/filmRadiationModel/filmRadiationModel/filmRadiationModel.C
  submodels/thermo/filmRadiationModel/filmRadiationModel/filmRadiationModelNew.C
  submodels/thermo/filmRadiationModel/noRadiation/noRadiation.C
  submodels/thermo/filmRadiationModel/constantRadiation/constantRadiation.C
  submodels/thermo/filmRadiationModel/primaryRadiation/primaryRadiation.C
  submodels/thermo/filmRadiationModel/standardRadiation/standardRadiation.C
  submodels/thermo/filmViscosityModel/filmViscosityModel/filmViscosityModel.C
  submodels/thermo/filmViscosityModel/filmViscosityModel/filmViscosityModelNew.C
  submodels/thermo/filmViscosityModel/constantViscosity/constantViscosity.C
  submodels/thermo/filmViscosityModel/liquidViscosity/liquidViscosity.C
  submodels/thermo/filmViscosityModel/thixotropicViscosity/thixotropicViscosity.C
  submodels/thermo/filmViscosityModel/ArrheniusViscosity/ArrheniusViscosity.C
  submodels/thermo/filmViscosityModel/function1Viscosity/function1Viscosity.C
  submodels/thermo/filmViscosityModel/waxSolventViscosity/waxSolventViscosity.C
  derivedFvPatchFields/filmHeightInletVelocity/filmHeightInletVelocityFvPatchVectorField.C
  derivedFvPatchFields/inclinedFilmNusseltHeight/inclinedFilmNusseltHeightFvPatchScalarField.C
  derivedFvPatchFields/inclinedFilmNusseltInletVelocity/inclinedFilmNusseltInletVelocityFvPatchVectorField.C
  functionObjects/filmFlux/filmFlux.C
)
add_library(surfaceFilmModels ${_FILES})
target_compile_features(surfaceFilmModels PUBLIC cxx_std_11)
set_property(TARGET surfaceFilmModels PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(surfaceFilmModels PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(surfaceFilmModels PUBLIC SLGThermo distributionModels regionModels)
target_include_directories(surfaceFilmModels PUBLIC
  .
)
install(TARGETS surfaceFilmModels DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
