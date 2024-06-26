/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2013-2017 OpenFOAM Foundation
    Copyright (C) 2020-2023 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "TurbulenceModels/phaseCompressible/RAS/mixtureKEpsilon/mixtureKEpsilon.H"
#include "cfdTools/general/fvOptions/fvOptions.H"
#include "cfdTools/general/bound/bound.H"
#include "twoPhaseSystem.H"
#include "interfacialModels/dragModels/dragModel/dragModel.H"
#include "interfacialModels/virtualMassModels/virtualMassModel/virtualMassModel.H"
#include "fields/fvPatchFields/basic/fixedValue/fixedValueFvPatchFields.H"
#include "fields/fvPatchFields/derived/inletOutlet/inletOutletFvPatchFields.H"
#include "finiteVolume/fvm/fvmSup.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace RASModels
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class BasicTurbulenceModel>
mixtureKEpsilon<BasicTurbulenceModel>::mixtureKEpsilon
(
    const alphaField& alpha,
    const rhoField& rho,
    const volVectorField& U,
    const surfaceScalarField& alphaRhoPhi,
    const surfaceScalarField& phi,
    const transportModel& transport,
    const word& propertiesName,
    const word& type
)
:
    eddyViscosity<RASModel<BasicTurbulenceModel>>
    (
        type,
        alpha,
        rho,
        U,
        alphaRhoPhi,
        phi,
        transport,
        propertiesName
    ),

    liquidTurbulencePtr_(nullptr),

    Cmu_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "Cmu",
            this->coeffDict_,
            0.09
        )
    ),
    C1_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "C1",
            this->coeffDict_,
            1.44
        )
    ),
    C2_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "C2",
            this->coeffDict_,
            1.92
        )
    ),
    C3_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "C3",
            this->coeffDict_,
            C2_.value()
        )
    ),
    Cp_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "Cp",
            this->coeffDict_,
            0.25
        )
    ),
    sigmak_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "sigmak",
            this->coeffDict_,
            1.0
        )
    ),
    sigmaEps_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "sigmaEps",
            this->coeffDict_,
            1.3
        )
    ),

    k_
    (
        IOobject
        (
            IOobject::groupName("k", alphaRhoPhi.group()),
            this->runTime_.timeName(),
            this->mesh_,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        this->mesh_
    ),
    epsilon_
    (
        IOobject
        (
            IOobject::groupName("epsilon", alphaRhoPhi.group()),
            this->runTime_.timeName(),
            this->mesh_,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        this->mesh_
    )
{
    bound(k_, this->kMin_);
    bound(epsilon_, this->epsilonMin_);

    if (type == typeName)
    {
        this->printCoeffs(type);
    }
}


template<class BasicTurbulenceModel>
wordList mixtureKEpsilon<BasicTurbulenceModel>::epsilonBoundaryTypes
(
    const volScalarField& epsilon
) const
{
    const volScalarField::Boundary& ebf = epsilon.boundaryField();

    wordList ebt = ebf.types();

    forAll(ebf, patchi)
    {
        if (isA<fixedValueFvPatchScalarField>(ebf[patchi]))
        {
            ebt[patchi] = fixedValueFvPatchScalarField::typeName;
        }
    }

    return ebt;
}


template<class BasicTurbulenceModel>
void mixtureKEpsilon<BasicTurbulenceModel>::correctInletOutlet
(
    volScalarField& vsf,
    const volScalarField& refVsf
) const
{
    volScalarField::Boundary& bf = vsf.boundaryFieldRef();
    const volScalarField::Boundary& refBf =
        refVsf.boundaryField();

    forAll(bf, patchi)
    {
        if
        (
            isA<inletOutletFvPatchScalarField>(bf[patchi])
         && isA<inletOutletFvPatchScalarField>(refBf[patchi])
        )
        {
            refCast<inletOutletFvPatchScalarField>
            (bf[patchi]).refValue() =
            refCast<const inletOutletFvPatchScalarField>
            (refBf[patchi]).refValue();
        }
    }
}


template<class BasicTurbulenceModel>
void mixtureKEpsilon<BasicTurbulenceModel>::initMixtureFields()
{
    if (rhom_) return;

    // Local references to gas-phase properties
    const volScalarField& kg = this->k_;
    const volScalarField& epsilong = this->epsilon_;

    // Local references to liquid-phase properties
    mixtureKEpsilon<BasicTurbulenceModel>& turbc = this->liquidTurbulence();
    const volScalarField& kl = turbc.k_;
    const volScalarField& epsilonl = turbc.epsilon_;

    word startTimeName
    (
        this->runTime_.timeName(this->runTime_.startTime().value())
    );

    Ct2_.reset
    (
        new volScalarField
        (
            IOobject
            (
                "Ct2",
                startTimeName,
                this->mesh_,
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
            Ct2()
        )
    );

    rhom_.reset
    (
        new volScalarField
        (
            IOobject
            (
                "rhom",
                startTimeName,
                this->mesh_,
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
            rhom()
        )
    );

    km_.reset
    (
        new volScalarField
        (
            IOobject
            (
                "km",
                startTimeName,
                this->mesh_,
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
            mix(kl, kg),
            kl.boundaryField().types()
        )
    );
    correctInletOutlet(km_(), kl);

    epsilonm_.reset
    (
        new volScalarField
        (
            IOobject
            (
                "epsilonm",
                startTimeName,
                this->mesh_,
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
            mix(epsilonl, epsilong),
            epsilonBoundaryTypes(epsilonl)
        )
    );
    correctInletOutlet(epsilonm_(), epsilonl);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class BasicTurbulenceModel>
bool mixtureKEpsilon<BasicTurbulenceModel>::read()
{
    if (eddyViscosity<RASModel<BasicTurbulenceModel>>::read())
    {
        Cmu_.readIfPresent(this->coeffDict());
        C1_.readIfPresent(this->coeffDict());
        C2_.readIfPresent(this->coeffDict());
        C3_.readIfPresent(this->coeffDict());
        Cp_.readIfPresent(this->coeffDict());
        sigmak_.readIfPresent(this->coeffDict());
        sigmaEps_.readIfPresent(this->coeffDict());

        return true;
    }

    return false;
}


template<class BasicTurbulenceModel>
void mixtureKEpsilon<BasicTurbulenceModel>::correctNut()
{
    this->nut_ = Cmu_*sqr(k_)/epsilon_;
    this->nut_.correctBoundaryConditions();
    fv::options::New(this->mesh_).correct(this->nut_);

    BasicTurbulenceModel::correctNut();
}


template<class BasicTurbulenceModel>
mixtureKEpsilon<BasicTurbulenceModel>&
mixtureKEpsilon<BasicTurbulenceModel>::liquidTurbulence() const
{
    if (!liquidTurbulencePtr_)
    {
        const volVectorField& U = this->U_;

        const transportModel& gas = this->transport();
        const twoPhaseSystem& fluid =
            refCast<const twoPhaseSystem>(gas.fluid());
        const transportModel& liquid = fluid.otherPhase(gas);

        liquidTurbulencePtr_ =
           &const_cast<mixtureKEpsilon<BasicTurbulenceModel>&>
            (
                U.db().lookupObject<mixtureKEpsilon<BasicTurbulenceModel>>
                (
                    IOobject::groupName
                    (
                        turbulenceModel::propertiesName,
                        liquid.name()
                    )
                )
            );
    }

    return *liquidTurbulencePtr_;
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::Ct2() const
{
    const mixtureKEpsilon<BasicTurbulenceModel>& liquidTurbulence =
        this->liquidTurbulence();

    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());
    const transportModel& liquid = fluid.otherPhase(gas);

    const volScalarField& alphag = this->alpha_;

    volScalarField magUr(mag(liquidTurbulence.U() - this->U()));

    volScalarField beta
    (
        (6*this->Cmu_/(4*sqrt(3.0/2.0)))
       *fluid.Kd()/liquid.rho()
       *(liquidTurbulence.k_/liquidTurbulence.epsilon_)
    );
    volScalarField Ct0((3 + beta)/(1 + beta + 2*gas.rho()/liquid.rho()));
    volScalarField fAlphad((180 + (-4.71e3 + 4.26e4*alphag)*alphag)*alphag);

    return sqr(1 + (Ct0 - 1)*exp(-fAlphad));
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::rholEff() const
{
    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());
    return fluid.otherPhase(gas).rho();
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::rhogEff() const
{
    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());
    const virtualMassModel& virtualMass =
        fluid.lookupSubModel<virtualMassModel>(gas, fluid.otherPhase(gas));
    return
        gas.rho()
      + virtualMass.Cvm()*fluid.otherPhase(gas).rho();
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::rhom() const
{
    const volScalarField& alphag = this->alpha_;
    const volScalarField& alphal = this->liquidTurbulence().alpha_;

    return alphal*rholEff() + alphag*rhogEff();
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::mix
(
    const volScalarField& fc,
    const volScalarField& fd
) const
{
    const volScalarField& alphag = this->alpha_;
    const volScalarField& alphal = this->liquidTurbulence().alpha_;

    return (alphal*rholEff()*fc + alphag*rhogEff()*fd)/rhom_();
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::mixU
(
    const volScalarField& fc,
    const volScalarField& fd
) const
{
    const volScalarField& alphag = this->alpha_;
    const volScalarField& alphal = this->liquidTurbulence().alpha_;

    return
        (alphal*rholEff()*fc + alphag*rhogEff()*Ct2_()*fd)
       /(alphal*rholEff() + alphag*rhogEff()*Ct2_());
}


template<class BasicTurbulenceModel>
tmp<surfaceScalarField> mixtureKEpsilon<BasicTurbulenceModel>::mixFlux
(
    const surfaceScalarField& fc,
    const surfaceScalarField& fd
) const
{
    const volScalarField& alphag = this->alpha_;
    const volScalarField& alphal = this->liquidTurbulence().alpha_;

    surfaceScalarField alphalf(fvc::interpolate(alphal));
    surfaceScalarField alphagf(fvc::interpolate(alphag));

    surfaceScalarField rholEfff(fvc::interpolate(rholEff()));
    surfaceScalarField rhogEfff(fvc::interpolate(rhogEff()));

    return
       (alphalf*rholEfff*fc + alphagf*rhogEfff*fvc::interpolate(Ct2_())*fd)
      /(alphalf*rholEfff + alphagf*rhogEfff*fvc::interpolate(Ct2_()));
}


template<class BasicTurbulenceModel>
tmp<volScalarField> mixtureKEpsilon<BasicTurbulenceModel>::bubbleG() const
{
    const mixtureKEpsilon<BasicTurbulenceModel>& liquidTurbulence =
        this->liquidTurbulence();

    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());
    const transportModel& liquid = fluid.otherPhase(gas);

    const dragModel& drag = fluid.lookupSubModel<dragModel>(gas, liquid);

    volScalarField magUr(mag(liquidTurbulence.U() - this->U()));

    // Lahey model
    tmp<volScalarField> bubbleG
    (
        Cp_
       *liquid*liquid.rho()
       *(
            pow3(magUr)
          + pow(drag.CdRe()*liquid.nu()/gas.d(), 4.0/3.0)
           *pow(magUr, 5.0/3.0)
        )
       *gas
       /gas.d()
    );

    // Simple model
    // tmp<volScalarField> bubbleG
    // (
    //     Cp_*liquid*drag.K()*sqr(magUr)
    // );

    return bubbleG;
}


template<class BasicTurbulenceModel>
tmp<fvScalarMatrix> mixtureKEpsilon<BasicTurbulenceModel>::kSource() const
{
    return fvm::Su(bubbleG()/rhom_(), km_());
}


template<class BasicTurbulenceModel>
tmp<fvScalarMatrix> mixtureKEpsilon<BasicTurbulenceModel>::epsilonSource() const
{
    return fvm::Su(C3_*epsilonm_()*bubbleG()/(rhom_()*km_()), epsilonm_());
}


template<class BasicTurbulenceModel>
void mixtureKEpsilon<BasicTurbulenceModel>::correct()
{
    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());

    // Only solve the mixture turbulence for the gas-phase
    if (&gas != &fluid.phase1())
    {
        // This is the liquid phase but check the model for the gas-phase
        // is consistent
        this->liquidTurbulence();

        return;
    }

    if (!this->turbulence_)
    {
        return;
    }

    // Initialise the mixture fields if they have not yet been constructed
    initMixtureFields();

    // Local references to gas-phase properties
    tmp<surfaceScalarField> phig = this->phi();
    const volVectorField& Ug = this->U_;
    const volScalarField& alphag = this->alpha_;
    volScalarField& kg = this->k_;
    volScalarField& epsilong = this->epsilon_;
    volScalarField& nutg = this->nut_;

    // Local references to liquid-phase properties
    mixtureKEpsilon<BasicTurbulenceModel>& liquidTurbulence =
        this->liquidTurbulence();
    tmp<surfaceScalarField> phil = liquidTurbulence.phi();
    const volVectorField& Ul = liquidTurbulence.U_;
    const volScalarField& alphal = liquidTurbulence.alpha_;
    volScalarField& kl = liquidTurbulence.k_;
    volScalarField& epsilonl = liquidTurbulence.epsilon_;
    volScalarField& nutl = liquidTurbulence.nut_;

    // Local references to mixture properties
    volScalarField& rhom = rhom_();
    volScalarField& km = km_();
    volScalarField& epsilonm = epsilonm_();

    fv::options& fvOptions(fv::options::New(this->mesh_));

    eddyViscosity<RASModel<BasicTurbulenceModel>>::correct();

    // Update the effective mixture density
    rhom = this->rhom();

    // Mixture flux
    surfaceScalarField phim("phim", mixFlux(phil, phig));

    // Mixture velocity divergence
    volScalarField divUm
    (
        mixU
        (
            fvc::div(fvc::absolute(phil, Ul)),
            fvc::div(fvc::absolute(phig, Ug))
        )
    );

    tmp<volScalarField> Gc;
    {
        tmp<volTensorField> tgradUl = fvc::grad(Ul);
        Gc = tmp<volScalarField>
        (
            new volScalarField
            (
                this->GName(),
                nutl*(tgradUl() && devTwoSymm(tgradUl()))
            )
        );
        tgradUl.clear();

        // Update k, epsilon and G at the wall
        kl.boundaryFieldRef().updateCoeffs();
        // Push any changed cell values to coupled neighbours
        kl.boundaryFieldRef().evaluateCoupled<coupledFvPatch>();

        epsilonl.boundaryFieldRef().updateCoeffs();
        epsilonl.boundaryFieldRef().evaluateCoupled<coupledFvPatch>();

        Gc.ref().checkOut();
    }

    tmp<volScalarField> Gd;
    {
        tmp<volTensorField> tgradUg = fvc::grad(Ug);
        Gd = tmp<volScalarField>
        (
            new volScalarField
            (
                this->GName(),
                nutg*(tgradUg() && devTwoSymm(tgradUg()))
            )
        );
        tgradUg.clear();

        // Update k, epsilon and G at the wall
        kg.boundaryFieldRef().updateCoeffs();
        // Push any changed cell values to coupled neighbours
        kg.boundaryFieldRef().evaluateCoupled<coupledFvPatch>();
        epsilong.boundaryFieldRef().updateCoeffs();
        // Push any changed cell values to coupled neighbours
        epsilong.boundaryFieldRef().evaluateCoupled<coupledFvPatch>();

        Gd.ref().checkOut();
    }

    // Mixture turbulence generation
    volScalarField Gm(mix(Gc, Gd));

    // Mixture turbulence viscosity
    volScalarField nutm(mixU(nutl, nutg));

    // Update the mixture k and epsilon boundary conditions
    km == mix(kl, kg);
    bound(km, this->kMin_);
    epsilonm == mix(epsilonl, epsilong);
    bound(epsilonm, this->epsilonMin_);

    // Dissipation equation
    tmp<fvScalarMatrix> epsEqn
    (
        fvm::ddt(epsilonm)
      + fvm::div(phim, epsilonm)
      - fvm::Sp(fvc::div(phim), epsilonm)
      - fvm::laplacian(DepsilonEff(nutm), epsilonm)
     ==
        C1_*Gm*epsilonm/km
      - fvm::SuSp(((2.0/3.0)*C1_)*divUm, epsilonm)
      - fvm::Sp(C2_*epsilonm/km, epsilonm)
      + epsilonSource()
      + fvOptions(epsilonm)
    );

    epsEqn.ref().relax();
    fvOptions.constrain(epsEqn.ref());
    epsEqn.ref().boundaryManipulate(epsilonm.boundaryFieldRef());
    solve(epsEqn);
    fvOptions.correct(epsilonm);
    bound(epsilonm, this->epsilonMin_);


    // Turbulent kinetic energy equation
    tmp<fvScalarMatrix> kmEqn
    (
        fvm::ddt(km)
      + fvm::div(phim, km)
      - fvm::Sp(fvc::div(phim), km)
      - fvm::laplacian(DkEff(nutm), km)
     ==
        Gm
      - fvm::SuSp((2.0/3.0)*divUm, km)
      - fvm::Sp(epsilonm/km, km)
      + kSource()
      + fvOptions(km)
    );

    kmEqn.ref().relax();
    fvOptions.constrain(kmEqn.ref());
    solve(kmEqn);
    fvOptions.correct(km);
    bound(km, this->kMin_);
    km.correctBoundaryConditions();

    volScalarField Cc2(rhom/(alphal*rholEff() + alphag*rhogEff()*Ct2_()));
    kl = Cc2*km;
    kl.correctBoundaryConditions();
    epsilonl = Cc2*epsilonm;
    epsilonl.correctBoundaryConditions();
    liquidTurbulence.correctNut();

    Ct2_() = Ct2();
    kg = Ct2_()*kl;
    kg.correctBoundaryConditions();
    epsilong = Ct2_()*epsilonl;
    epsilong.correctBoundaryConditions();
    nutg = Ct2_()*(liquidTurbulence.nu()/this->nu())*nutl;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace Foam

// ************************************************************************* //
