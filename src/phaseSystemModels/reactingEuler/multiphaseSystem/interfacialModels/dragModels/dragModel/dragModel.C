/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2018 OpenFOAM Foundation
    Copyright (C) 2019-2021 OpenCFD Ltd.
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

#include "interfacialModels/dragModels/dragModel/dragModel.H"
#include "phasePair/reactingEuler_phasePair.H"
#include "interfacialModels/swarmCorrections/swarmCorrection/swarmCorrection.H"
#include "interpolation/surfaceInterpolation/surfaceInterpolation/surfaceInterpolate.H"
#include "BlendedInterfacialModel/BlendedInterfacialModel.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(dragModel, 0);
    defineBlendedInterfacialModelTypeNameAndDebug(dragModel, 0);
    defineRunTimeSelectionTable(dragModel, dictionary);
}

const Foam::dimensionSet Foam::dragModel::dimK(1, -3, -1, 0, 0);


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dragModel::dragModel
(
    const phasePair& pair,
    const bool registerObject
)
:
    regIOobject
    (
        IOobject
        (
            IOobject::groupName(typeName, pair.name()),
            pair.phase1().mesh().time().timeName(),
            pair.phase1().mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            registerObject
        )
    ),
    pair_(pair)
{}


Foam::dragModel::dragModel
(
    const dictionary& dict,
    const phasePair& pair,
    const bool registerObject
)
:
    regIOobject
    (
        IOobject
        (
            IOobject::groupName(typeName, pair.name()),
            pair.phase1().mesh().time().timeName(),
            pair.phase1().mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            registerObject
        )
    ),
    pair_(pair),
    swarmCorrection_
    (
        swarmCorrection::New
        (
            dict.subDict("swarmCorrection"),
            pair
        )
    )
{}


// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

Foam::autoPtr<Foam::dragModel>
Foam::dragModel::New
(
    const dictionary& dict,
    const phasePair& pair
)
{
    const word modelType(dict.get<word>("type"));

    Info<< "Selecting dragModel for "
        << pair << ": " << modelType << endl;

    auto* ctorPtr = dictionaryConstructorTable(modelType);

    if (!ctorPtr)
    {
        FatalIOErrorInLookup
        (
            dict,
            "dragModel",
            modelType,
            *dictionaryConstructorTablePtr_
        ) << abort(FatalIOError);
    }

    return ctorPtr(dict, pair, true);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::dragModel::~dragModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField> Foam::dragModel::Ki() const
{
    return
        0.75
       *CdRe()
       *swarmCorrection_->Cs()
       *pair_.continuous().rho()
       *pair_.continuous().nu()
       /sqr(pair_.dispersed().d());
}


Foam::tmp<Foam::volScalarField> Foam::dragModel::K() const
{
    return max(pair_.dispersed(), pair_.dispersed().residualAlpha())*Ki();
}


Foam::tmp<Foam::surfaceScalarField> Foam::dragModel::Kf() const
{
    return
        max
        (
            fvc::interpolate(pair_.dispersed()),
            pair_.dispersed().residualAlpha()
        )*fvc::interpolate(Ki());
}


bool Foam::dragModel::writeData(Ostream& os) const
{
    return os.good();
}


// ************************************************************************* //
