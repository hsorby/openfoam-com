/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2014-2017 OpenFOAM Foundation
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

#include "interfacialModels/wallLubricationModels/TomiyamaWallLubrication/TomiyamaWallLubrication.H"
#include "phasePair/phasePair.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace wallLubricationModels
{
    defineTypeNameAndDebug(TomiyamaWallLubrication, 0);
    addToRunTimeSelectionTable
    (
        wallLubricationModel,
        TomiyamaWallLubrication,
        dictionary
    );
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::wallLubricationModels::TomiyamaWallLubrication::TomiyamaWallLubrication
(
    const dictionary& dict,
    const phasePair& pair
)
:
    wallLubricationModel(dict, pair),
    D_("Cwd", dimLength, dict)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::wallLubricationModels::TomiyamaWallLubrication::~TomiyamaWallLubrication()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volVectorField>
Foam::wallLubricationModels::TomiyamaWallLubrication::Fi() const
{
    volVectorField Ur(pair_.Ur());

    const volVectorField& n(nWall());
    const volScalarField& y(yWall());

    volScalarField Eo(pair_.Eo());

    return
        (
            pos0(Eo - 1.0)*neg(Eo - 5.0)*exp(-0.933*Eo + 0.179)
          + pos0(Eo - 5.0)*neg(Eo - 33.0)*(0.00599*Eo - 0.0187)
          + pos0(Eo - 33.0)*0.179
        )
       *0.5
       *pair_.dispersed().d()
       *(
            1/sqr(y)
          - 1/sqr(D_ - y)
        )
       *pair_.continuous().rho()
       *magSqr(Ur - (Ur & n)*n)
       *n;
}


// ************************************************************************* //
