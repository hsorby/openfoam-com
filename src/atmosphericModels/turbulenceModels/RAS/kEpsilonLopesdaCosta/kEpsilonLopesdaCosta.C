/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2018 OpenFOAM Foundation
    Copyright (C) 2018-2023 OpenCFD Ltd.
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

#include "turbulenceModels/RAS/kEpsilonLopesdaCosta/kEpsilonLopesdaCosta.H"
#include "cfdTools/general/fvOptions/fvOptions.H"
#include "sources/derived/explicitPorositySource/explicitPorositySource.H"
#include "cfdTools/general/bound/bound.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace RASModels
{

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class BasicTurbulenceModel>
void kEpsilonLopesdaCosta<BasicTurbulenceModel>::setPorosityCoefficient
(
    volScalarField::Internal& C,
    const porosityModels::powerLawLopesdaCosta& pm
)
{
    if (pm.dict().found(C.name()))
    {
        const labelList& cellZoneIDs = pm.cellZoneIDs();

        const scalar Cpm = pm.dict().get<scalar>(C.name());

        for (const label zonei : cellZoneIDs)
        {
            const labelList& cells = this->mesh_.cellZones()[zonei];

            for (const label celli : cells)
            {
                C[celli] = Cpm;
            }
        }
    }
}


template<class BasicTurbulenceModel>
void kEpsilonLopesdaCosta<BasicTurbulenceModel>::setCdSigma
(
    volScalarField::Internal& C,
    const porosityModels::powerLawLopesdaCosta& pm
)
{
    if (pm.dict().found(C.name()))
    {
        const labelList& cellZoneIDs = pm.cellZoneIDs();
        const scalarField& Sigma = pm.Sigma();

        const scalar Cpm = pm.dict().get<scalar>(C.name());

        for (const label zonei : cellZoneIDs)
        {
            const labelList& cells = this->mesh_.cellZones()[zonei];

            forAll(cells, i)
            {
                const label celli = cells[i];
                C[celli] = Cpm*Sigma[i];
            }
        }
    }
}


template<class BasicTurbulenceModel>
void kEpsilonLopesdaCosta<BasicTurbulenceModel>::setPorosityCoefficients()
{
    fv::options::optionList& fvOptions(fv::options::New(this->mesh_));

    forAll(fvOptions, i)
    {
        if (isA<fv::explicitPorositySource>(fvOptions[i]))
        {
            const fv::explicitPorositySource& eps =
                refCast<const fv::explicitPorositySource>(fvOptions[i]);

            if (isA<porosityModels::powerLawLopesdaCosta>(eps.model()))
            {
                const porosityModels::powerLawLopesdaCosta& pm =
                    refCast<const porosityModels::powerLawLopesdaCosta>
                    (
                        eps.model()
                    );

                setPorosityCoefficient(Cmu_, pm);
                Cmu_.correctBoundaryConditions();
                setPorosityCoefficient(C1_, pm);
                setPorosityCoefficient(C2_, pm);
                setPorosityCoefficient(sigmak_, pm);
                setPorosityCoefficient(sigmaEps_, pm);
                sigmak_.correctBoundaryConditions();
                sigmaEps_.correctBoundaryConditions();

                setCdSigma(CdSigma_, pm);
                setPorosityCoefficient(betap_, pm);
                setPorosityCoefficient(betad_, pm);
                setPorosityCoefficient(C4_, pm);
                setPorosityCoefficient(C5_, pm);
            }
        }
    }
}


template<class BasicTurbulenceModel>
void kEpsilonLopesdaCosta<BasicTurbulenceModel>::correctNut()
{
    this->nut_ = Cmu_*sqr(k_)/epsilon_;
    this->nut_.correctBoundaryConditions();
    fv::options::New(this->mesh_).correct(this->nut_);

    BasicTurbulenceModel::correctNut();
}


template<class BasicTurbulenceModel>
tmp<fvScalarMatrix> kEpsilonLopesdaCosta<BasicTurbulenceModel>::kSource
(
    const volScalarField::Internal& magU,
    const volScalarField::Internal& magU3
) const
{
    return fvm::Su(CdSigma_*(betap_*magU3 - betad_*magU*k_()), k_);
}


template<class BasicTurbulenceModel>
tmp<fvScalarMatrix>
kEpsilonLopesdaCosta<BasicTurbulenceModel>::epsilonSource
(
    const volScalarField::Internal& magU,
    const volScalarField::Internal& magU3
) const
{
    return fvm::Su
    (
        CdSigma_
       *(C4_*betap_*epsilon_()/k_()*magU3 - C5_*betad_*magU*epsilon_()),
        epsilon_
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class BasicTurbulenceModel>
kEpsilonLopesdaCosta<BasicTurbulenceModel>::kEpsilonLopesdaCosta
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

    Cmu_
    (
        IOobject
        (
            "Cmu",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensioned<scalar>::getOrAddToDict
        (
            "Cmu",
            this->coeffDict_,
            0.09
        )
    ),
    C1_
    (
        IOobject
        (
            "C1",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensioned<scalar>::getOrAddToDict
        (
            "C1",
            this->coeffDict_,
            1.44
        )
    ),
    C2_
    (
        IOobject
        (
            "C2",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensioned<scalar>::getOrAddToDict
        (
            "C2",
            this->coeffDict_,
            1.92
        )
    ),
    sigmak_
    (
        IOobject
        (
            "sigmak",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensioned<scalar>::getOrAddToDict
        (
            "sigmak",
            this->coeffDict_,
            1.0
        )
    ),
    sigmaEps_
    (
        IOobject
        (
            "sigmaEps",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensioned<scalar>::getOrAddToDict
        (
            "sigmaEps",
            this->coeffDict_,
            1.3
        )
    ),

    CdSigma_
    (
        IOobject
        (
            "CdSigma",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensionedScalar("CdSigma", dimless/dimLength, 0)
    ),
    betap_
    (
        IOobject
        (
            "betap",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensionedScalar("betap", dimless, 0)
    ),
    betad_
    (
        IOobject
        (
            "betad",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensionedScalar("betad", dimless, 0)
    ),
    C4_
    (
        IOobject
        (
            "C4",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensionedScalar("C4", dimless, 0)
    ),
    C5_
    (
        IOobject
        (
            "C5",
            this->runTime_.timeName(),
            this->mesh_
        ),
        this->mesh_,
        dimensionedScalar("C5", dimless, 0)
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

    setPorosityCoefficients();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class BasicTurbulenceModel>
bool kEpsilonLopesdaCosta<BasicTurbulenceModel>::read()
{
    if (eddyViscosity<RASModel<BasicTurbulenceModel>>::read())
    {
        return true;
    }

    return false;
}


template<class BasicTurbulenceModel>
void kEpsilonLopesdaCosta<BasicTurbulenceModel>::correct()
{
    if (!this->turbulence_)
    {
        return;
    }

    // Local references
    const alphaField& alpha = this->alpha_;
    const rhoField& rho = this->rho_;
    const surfaceScalarField& alphaRhoPhi = this->alphaRhoPhi_;
    const volVectorField& U = this->U_;
    volScalarField& nut = this->nut_;
    fv::options& fvOptions(fv::options::New(this->mesh_));

    eddyViscosity<RASModel<BasicTurbulenceModel>>::correct();

    volScalarField::Internal divU
    (
        fvc::div(fvc::absolute(this->phi(), U))().v()
    );

    tmp<volTensorField> tgradU = fvc::grad(U);
    volScalarField::Internal G
    (
        this->GName(),
        nut.v()*(devTwoSymm(tgradU().v()) && tgradU().v())
    );
    tgradU.clear();

    // Update epsilon and G at the wall
    epsilon_.boundaryFieldRef().updateCoeffs();
    // Push any changed cell values to coupled neighbours
    epsilon_.boundaryFieldRef().template evaluateCoupled<coupledFvPatch>();

    volScalarField::Internal magU(mag(U));
    volScalarField::Internal magU3(pow3(magU));

    // Dissipation equation
    tmp<fvScalarMatrix> epsEqn
    (
        fvm::ddt(alpha, rho, epsilon_)
      + fvm::div(alphaRhoPhi, epsilon_)
      - fvm::laplacian(alpha*rho*DepsilonEff(), epsilon_)
     ==
        C1_*alpha()*rho()*G*epsilon_()/k_()
      - fvm::SuSp(((2.0/3.0)*C1_)*alpha()*rho()*divU, epsilon_)
      - fvm::Sp(C2_*alpha()*rho()*epsilon_()/k_(), epsilon_)
      + epsilonSource(magU, magU3)
      + fvOptions(alpha, rho, epsilon_)
    );

    epsEqn.ref().relax();
    fvOptions.constrain(epsEqn.ref());
    epsEqn.ref().boundaryManipulate(epsilon_.boundaryFieldRef());
    solve(epsEqn);
    fvOptions.correct(epsilon_);
    bound(epsilon_, this->epsilonMin_);

    // Turbulent kinetic energy equation
    tmp<fvScalarMatrix> kEqn
    (
        fvm::ddt(alpha, rho, k_)
      + fvm::div(alphaRhoPhi, k_)
      - fvm::laplacian(alpha*rho*DkEff(), k_)
     ==
        alpha()*rho()*G
      - fvm::SuSp((2.0/3.0)*alpha()*rho()*divU, k_)
      - fvm::Sp(alpha()*rho()*epsilon_()/k_(), k_)
      + kSource(magU, magU3)
      + fvOptions(alpha, rho, k_)
    );

    kEqn.ref().relax();
    fvOptions.constrain(kEqn.ref());
    solve(kEqn);
    fvOptions.correct(k_);
    bound(k_, this->kMin_);

    correctNut();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace Foam

// ************************************************************************* //
