/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2019-2020,2023 OpenCFD Ltd.
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

#include "PDRModels/turbulence/PDRkEpsilon/PDRkEpsilon.H"
#include "PDRModels/dragModels/PDRDragModel/PDRDragModel.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace compressible
{
namespace RASModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(PDRkEpsilon, 0);
addToRunTimeSelectionTable(RASModel, PDRkEpsilon, dictionary);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

PDRkEpsilon::PDRkEpsilon
(
    const geometricOneField& alpha,
    const volScalarField& rho,
    const volVectorField& U,
    const surfaceScalarField& alphaRhoPhi,
    const surfaceScalarField& phi,
    const fluidThermo& thermophysicalModel,
    const word& turbulenceModelName,
    const word& modelName
)
:
    Foam::RASModels::kEpsilon<EddyDiffusivity<compressible::turbulenceModel>>
    (
        geometricOneField(),
        rho,
        U,
        phi,
        phi,
        thermophysicalModel,
        turbulenceModelName,
        modelName
    ),

    C4_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "C4",
            coeffDict_,
            0.1
        )
    )
{}


// * * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

PDRkEpsilon::~PDRkEpsilon()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool PDRkEpsilon::read()
{
    if (RASModel::read())
    {
        C4_.readIfPresent(coeffDict_);
        return true;
    }

    return false;
}


void PDRkEpsilon::correct()
{
    if (!turbulence_)
    {
        // Re-calculate viscosity
        nut_ = Cmu_*sqr(k_)/epsilon_;
        nut_.correctBoundaryConditions();

        // Re-calculate thermal diffusivity
        //***HGWalphat_ = mut_/Prt_;
        //alphat_.correctBoundaryConditions();

        return;
    }

    RASModel::correct();

    volScalarField divU(fvc::div(phi_/fvc::interpolate(rho_)));

    if (mesh_.moving())
    {
        divU += fvc::div(mesh_.phi());
    }

    tmp<volTensorField> tgradU = fvc::grad(U_);
    volScalarField G(GName(), rho_*nut_*(tgradU() && dev(twoSymm(tgradU()))));
    tgradU.clear();

    // Update epsilon and G at the wall
    epsilon_.boundaryFieldRef().updateCoeffs();
    // Push new cell values to
    // coupled neighbours. Note that we want to avoid the re-updateCoeffs
    // of the wallFunctions so make sure to bypass the evaluate on
    // those patches and only do the coupled ones.
    epsilon_.boundaryFieldRef().evaluateCoupled<coupledFvPatch>();

    // Add the blockage generation term so that it is included consistently
    // in both the k and epsilon equations
    const volScalarField& betav =
        U_.db().lookupObject<volScalarField>("betav");

    const volScalarField& Lobs =
        U_.db().lookupObject<volScalarField>("Lobs");

    const PDRDragModel& drag =
        U_.db().lookupObject<PDRDragModel>("PDRDragModel");

    volScalarField GR(drag.Gk());

    volScalarField LI
        (C4_*(Lobs + dimensionedScalar("minLength", dimLength, VSMALL)));

    // Dissipation equation
    tmp<fvScalarMatrix> epsEqn
    (
        betav*fvm::ddt(rho_, epsilon_)
      + fvm::div(phi_, epsilon_)
      - fvm::laplacian(rho_*DepsilonEff(), epsilon_)
     ==
        C1_*betav*G*epsilon_/k_
      + 1.5*pow(Cmu_, 3.0/4.0)*GR*sqrt(k_)/LI
      - fvm::SuSp(((2.0/3.0)*C1_)*betav*rho_*divU, epsilon_)
      - fvm::Sp(C2_*betav*rho_*epsilon_/k_, epsilon_)
    );

    epsEqn.ref().relax();

    epsEqn.ref().boundaryManipulate(epsilon_.boundaryFieldRef());

    solve(epsEqn);
    bound(epsilon_, epsilonMin_);


    // Turbulent kinetic energy equation

    tmp<fvScalarMatrix> kEqn
    (
        betav*fvm::ddt(rho_, k_)
      + fvm::div(phi_, k_)
      - fvm::laplacian(rho_*DkEff(), k_)
     ==
        betav*G + GR
      - fvm::SuSp((2.0/3.0)*betav*rho_*divU, k_)
      - fvm::Sp(betav*rho_*epsilon_/k_, k_)
    );

    kEqn.ref().relax();
    solve(kEqn);
    bound(k_, kMin_);

    // Re-calculate viscosity
    nut_ = Cmu_*sqr(k_)/epsilon_;
    nut_.correctBoundaryConditions();

    // Re-calculate thermal diffusivity
    //***HGWalphat_ = mut_/Prt_;
    //alphat_.correctBoundaryConditions();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace compressible
} // End namespace Foam

// ************************************************************************* //
