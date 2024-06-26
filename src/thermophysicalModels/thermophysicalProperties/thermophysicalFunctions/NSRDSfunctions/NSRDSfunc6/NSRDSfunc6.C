/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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

#include "thermophysicalFunctions/NSRDSfunctions/NSRDSfunc6/NSRDSfunc6.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(NSRDSfunc6, 0);
    addToRunTimeSelectionTable(thermophysicalFunction, NSRDSfunc6, dictionary);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::NSRDSfunc6::NSRDSfunc6
(
    const scalar Tc,
    const scalar a,
    const scalar b,
    const scalar c,
    const scalar d,
    const scalar e
)
:
    Tc_(Tc),
    a_(a),
    b_(b),
    c_(c),
    d_(d),
    e_(e)
{}


Foam::NSRDSfunc6::NSRDSfunc6(const dictionary& dict)
:
    Tc_(dict.get<scalar>("Tc")),
    a_(dict.get<scalar>("a")),
    b_(dict.get<scalar>("b")),
    c_(dict.get<scalar>("c")),
    d_(dict.get<scalar>("d")),
    e_(dict.get<scalar>("e"))
{}


// ************************************************************************* //
