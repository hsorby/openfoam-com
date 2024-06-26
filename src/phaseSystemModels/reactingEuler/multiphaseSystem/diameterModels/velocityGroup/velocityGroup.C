/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2017-2019 OpenFOAM Foundation
    Copyright (C) 2020 OpenCFD Ltd.
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

#include "diameterModels/velocityGroup/velocityGroup.H"
#include "diameterModels/velocityGroup/sizeGroup/sizeGroup.H"
#include "populationBalanceModel/populationBalanceModel/populationBalanceModel.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "fields/fvPatchFields/basic/zeroGradient/zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace diameterModels
{
    defineTypeNameAndDebug(velocityGroup, 0);

    addToRunTimeSelectionTable
    (
        diameterModel,
        velocityGroup,
        dictionary
    );
}
}


// * * * * * * * * * * * * Private Member Functions * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField> Foam::diameterModels::velocityGroup::dsm() const
{
    tmp<volScalarField> tInvDsm
    (
        volScalarField::New
        (
            "invDsm",
            phase_.mesh(),
            dimensionedScalar(inv(dimLength))
        )
    );

    volScalarField& invDsm = tInvDsm.ref();

    forAll(sizeGroups_, i)
    {
        const sizeGroup& fi = sizeGroups_[i];

        invDsm += fi/fi.d();
    }

    return 1.0/tInvDsm;
}


Foam::tmp<Foam::volScalarField>
Foam::diameterModels::velocityGroup::fSum() const
{
    tmp<volScalarField> tsumSizeGroups
    (
        volScalarField::New
        (
            "sumSizeGroups",
            phase_.mesh(),
            dimensionedScalar("zero", dimless, 0)
        )
    );

    volScalarField& sumSizeGroups = tsumSizeGroups.ref();

    forAll(sizeGroups_, i)
    {
        sumSizeGroups += sizeGroups_[i];
    }

    return tsumSizeGroups;
}


void Foam::diameterModels::velocityGroup::renormalize()
{
        Info<< "Renormalizing sizeGroups for velocityGroup "
            << phase_.name()
            << endl;

        // Set negative values to zero
        forAll(sizeGroups_, i)
        {
            sizeGroups_[i] *= pos(sizeGroups_[i]);
        };

        forAll(sizeGroups_, i)
        {
            sizeGroups_[i] /= fSum_;
        };
}


Foam::tmp<Foam::fv::convectionScheme<Foam::scalar>>
Foam::diameterModels::velocityGroup::mvconvection() const
{
    tmp<fv::convectionScheme<Foam::scalar>> mvConvection
    (
        fv::convectionScheme<Foam::scalar>::New
        (
            phase_.mesh(),
            fields_,
            phase_.alphaRhoPhi(),
            phase_.mesh().divScheme
            (
                "div(" + phase_.alphaRhoPhi()().name() + ",f)"
            )
        )
    );

    return mvConvection;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::diameterModels::velocityGroup::velocityGroup
(
    const dictionary& diameterProperties,
    const phaseModel& phase
)
:
    diameterModel(diameterProperties, phase),
    popBalName_(diameterProperties.lookup("populationBalance")),
    f_
    (
        IOobject
        (
            IOobject::groupName
            (
                "f",
                IOobject::groupName
                (
                    phase.name(),
                    popBalName_
                )
            ),
            phase.time().timeName(),
            phase.mesh(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        phase.mesh()
    ),
    formFactor_("formFactor", dimless, diameterProperties),
    sizeGroups_
    (
        diameterProperties.lookup("sizeGroups"),
        sizeGroup::iNew(phase, *this)
    ),
    fSum_
    (
        IOobject
        (
            IOobject::groupName
            (
                "fsum",
                IOobject::groupName
                (
                    phase.name(),
                    popBalName_
                )
            ),
            phase.time().timeName(),
            phase.mesh()
        ),
        fSum()
    ),
    d_
    (
        IOobject
        (
            IOobject::groupName("d", phase.name()),
            phase.time().timeName(),
            phase.mesh(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        phase.mesh(),
        dimensionedScalar(dimLength)
    ),
    dmdt_
    (
        IOobject
        (
            IOobject::groupName("source", phase.name()),
            phase.time().timeName(),
            phase.mesh()
        ),
        phase.mesh(),
        dimensionedScalar(dimDensity/dimTime)
    )
{
    if
    (
        phase_.mesh().solverDict(popBalName_).getOrDefault<Switch>
        (
            "renormalizeAtRestart",
            false
        )
     ||
        phase_.mesh().solverDict(popBalName_).getOrDefault<Switch>
        (
            "renormalize",
            false
        )
    )
    {
        renormalize();
    }

    fSum_ = fSum();

    if
    (
        mag(1 - fSum_.weightedAverage(fSum_.mesh().V()).value()) >= 1e-5
     || mag(1 - max(fSum_).value()) >= 1e-5
     || mag(1 - min(fSum_).value()) >= 1e-5
    )
    {
        FatalErrorInFunction
            << " Initial values of the sizeGroups belonging to velocityGroup "
            << this->phase().name()
            << " must add to" << nl << " unity. This condition might be"
            << " violated due to wrong entries in the" << nl
            << " velocityGroupCoeffs subdictionary or bad initial conditions in"
            << " the startTime" << nl
            << " directory. The sizeGroups can be renormalized at every"
            << " timestep or at restart" << nl
            << " only by setting the corresponding switch renormalize or"
            << " renormalizeAtRestart" << nl
            << " in the fvSolution subdictionary " << popBalName_ << "."
            << " Note that boundary conditions are not" << nl << "renormalized."
            << exit(FatalError);
    }

    forAll(sizeGroups_, i)
    {
        fields_.add(sizeGroups_[i]);
    }

    d_ = dsm();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::diameterModels::velocityGroup::~velocityGroup()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


void Foam::diameterModels::velocityGroup::preSolve()
{
    mvConvection_ = mvconvection();
}


void Foam::diameterModels::velocityGroup::postSolve()
{
    d_ = dsm();

    Info<< this->phase().name() << " Sauter mean diameter, min, max = "
        << d_.weightedAverage(d_.mesh().V()).value()
        << ' ' << min(d_).value()
        << ' ' << max(d_).value()
        << endl;

    fSum_ = fSum();

    Info<< phase_.name() << " sizeGroups-sum volume fraction, min, max = "
        << fSum_.weightedAverage(phase_.mesh().V()).value()
        << ' ' << min(fSum_).value()
        << ' ' << max(fSum_).value()
        << endl;

    if
    (
        phase_.mesh().solverDict(popBalName_).getOrDefault<Switch>
        (
            "renormalize",
            false
        )
    )
    {
        renormalize();
    }
}


bool Foam::diameterModels::velocityGroup::
read(const dictionary& phaseProperties)
{
    diameterModel::read(phaseProperties);

    return true;
}


Foam::tmp<Foam::volScalarField>
Foam::diameterModels::velocityGroup::d() const
{
    return d_;
}

// ************************************************************************* //
