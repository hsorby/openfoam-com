set(_FILES
  phaseModel/phaseModel.C
  phasePair/phasePairKey.C
  phasePair/phasePair.C
  phasePair/orderedPhasePair.C
  twoPhaseSystem/twoPhaseSystem.C
  diameterModels/diameterModel/diameterModel.C
  diameterModels/constantDiameter/constantDiameter.C
  diameterModels/isothermalDiameter/isothermalDiameter.C
  diameterModels/IATE/IATE.C
  diameterModels/IATE/IATEsources/IATEsource/IATEsource.C
  diameterModels/IATE/IATEsources/wakeEntrainmentCoalescence/wakeEntrainmentCoalescence.C
  diameterModels/IATE/IATEsources/turbulentBreakUp/turbulentBreakUp.C
  diameterModels/IATE/IATEsources/randomCoalescence/randomCoalescence.C
  BlendedInterfacialModel/blendingMethods/blendingMethod/blendingMethod.C
  BlendedInterfacialModel/blendingMethods/noBlending/noBlending.C
  BlendedInterfacialModel/blendingMethods/linear/linear.C
  BlendedInterfacialModel/blendingMethods/hyperbolic/hyperbolic.C
  interfacialModels/wallDependentModel/wallDependentModel.C
  interfacialModels/aspectRatioModels/aspectRatioModel/aspectRatioModel.C
  interfacialModels/aspectRatioModels/constantAspectRatio/constantAspectRatio.C
  interfacialModels/aspectRatioModels/TomiyamaAspectRatio/TomiyamaAspectRatio.C
  interfacialModels/aspectRatioModels/VakhrushevEfremov/VakhrushevEfremov.C
  interfacialModels/aspectRatioModels/Wellek/Wellek.C
  interfacialModels/dragModels/dragModel/dragModel.C
  interfacialModels/dragModels/segregated/segregated.C
  interfacialModels/dragModels/Ergun/Ergun.C
  interfacialModels/dragModels/Gibilaro/Gibilaro.C
  interfacialModels/dragModels/GidaspowErgunWenYu/GidaspowErgunWenYu.C
  interfacialModels/dragModels/GidaspowSchillerNaumann/GidaspowSchillerNaumann.C
  interfacialModels/dragModels/Lain/Lain.C
  interfacialModels/dragModels/SchillerNaumann/SchillerNaumann.C
  interfacialModels/dragModels/SyamlalOBrien/SyamlalOBrien.C
  interfacialModels/dragModels/TomiyamaCorrelated/TomiyamaCorrelated.C
  interfacialModels/dragModels/TomiyamaAnalytic/TomiyamaAnalytic.C
  interfacialModels/dragModels/WenYu/WenYu.C
  interfacialModels/dragModels/IshiiZuber/IshiiZuber.C
  interfacialModels/liftModels/liftModel/liftModel.C
  interfacialModels/liftModels/noLift/noLift.C
  interfacialModels/liftModels/constantLiftCoefficient/constantLiftCoefficient.C
  interfacialModels/liftModels/Moraga/Moraga.C
  interfacialModels/liftModels/LegendreMagnaudet/LegendreMagnaudet.C
  interfacialModels/liftModels/TomiyamaLift/TomiyamaLift.C
  interfacialModels/heatTransferModels/heatTransferModel/heatTransferModel.C
  interfacialModels/heatTransferModels/RanzMarshall/RanzMarshall.C
  interfacialModels/heatTransferModels/sphericalHeatTransfer/sphericalHeatTransfer.C
  interfacialModels/swarmCorrections/swarmCorrection/swarmCorrection.C
  interfacialModels/swarmCorrections/noSwarm/noSwarm.C
  interfacialModels/swarmCorrections/TomiyamaSwarm/TomiyamaSwarm.C
  interfacialModels/turbulentDispersionModels/turbulentDispersionModel/turbulentDispersionModel.C
  interfacialModels/turbulentDispersionModels/noTurbulentDispersion/noTurbulentDispersion.C
  interfacialModels/turbulentDispersionModels/constantTurbulentDispersionCoefficient/constantTurbulentDispersionCoefficient.C
  interfacialModels/turbulentDispersionModels/Burns/Burns.C
  interfacialModels/turbulentDispersionModels/Gosman/Gosman.C
  interfacialModels/turbulentDispersionModels/LopezDeBertodano/LopezDeBertodano.C
  interfacialModels/virtualMassModels/virtualMassModel/virtualMassModel.C
  interfacialModels/virtualMassModels/noVirtualMass/noVirtualMass.C
  interfacialModels/virtualMassModels/constantVirtualMassCoefficient/constantVirtualMassCoefficient.C
  interfacialModels/virtualMassModels/Lamb/Lamb.C
  interfacialModels/wallLubricationModels/wallLubricationModel/wallLubricationModel.C
  interfacialModels/wallLubricationModels/noWallLubrication/noWallLubrication.C
  interfacialModels/wallLubricationModels/Antal/Antal.C
  interfacialModels/wallLubricationModels/Frank/Frank.C
  interfacialModels/wallLubricationModels/TomiyamaWallLubrication/TomiyamaWallLubrication.C
  turbulence/phaseCompressibleTurbulenceModels.C
)
add_library(compressibleTwoPhaseSystem ${_FILES})
target_compile_features(compressibleTwoPhaseSystem PUBLIC cxx_std_11)
set_property(TARGET compressibleTwoPhaseSystem PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(compressibleTwoPhaseSystem PUBLIC WM_LABEL_SIZE=${WM_LABEL_SIZE} WM_${WM_PRECISION} NoRepository OPENFOAM=${OPENFOAM_VERSION})
target_link_libraries(compressibleTwoPhaseSystem PUBLIC compressibleTurbulenceModels incompressibleTransportModels)
target_include_directories(compressibleTwoPhaseSystem PUBLIC
  .
  PRIVATE
  ${OPENFOAM_SRC_DIR}/
)
install(TARGETS compressibleTwoPhaseSystem DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfoam EXPORT openfoam-targets)
