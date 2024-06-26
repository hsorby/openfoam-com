/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2019 OpenCFD Ltd.
    Copyright (C) 2019-2020 DLR
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

#include "implicitFunctions/cylinder/cylinderImplicitFunction.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    namespace implicitFunctions
    {
        defineTypeNameAndDebug(cylinderImplicitFunction, 0);
        addToRunTimeSelectionTable
        (
            implicitFunction,
            cylinderImplicitFunction,
            dict
        );
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::implicitFunctions::cylinderImplicitFunction::cylinderImplicitFunction
(
    const point& origin,
    const scalar radius,
    const scalar scale,
    const vector& direction
)
:
    origin_(origin),
    radius_(radius),
    scale_(scale),
    direction_(normalised(direction)),
    project_(tensor::I - direction_*direction_) // outer product
{}


Foam::implicitFunctions::cylinderImplicitFunction::cylinderImplicitFunction
(
    const dictionary& dict
)
:
    // __INTEL_COMPILER bug with inheriting constructors?? (issue #1821)
    origin_(dict.get<point>("origin")),
    radius_(dict.get<scalar>("radius")),
    scale_(dict.getOrDefault<scalar>("scale", 1)),
    direction_(dict.get<vector>("direction").normalise()),
    project_(tensor::I - direction_*direction_) // outer product
{}


// ************************************************************************* //
