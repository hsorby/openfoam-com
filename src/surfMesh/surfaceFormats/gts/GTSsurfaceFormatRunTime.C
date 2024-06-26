/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011 OpenFOAM Foundation
    Copyright (C) 2016 OpenCFD Ltd.
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

#include "surfaceFormats/gts/GTSsurfaceFormat.H"

#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "db/runTimeSelection/memberFunctions/addToMemberFunctionSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace fileFormats
{

// read UnsortedMeshedSurface
addNamedTemplatedToRunTimeSelectionTable
(
    UnsortedMeshedSurface,
    GTSsurfaceFormat,
    face,
    fileExtension,
    gts
);
addNamedTemplatedToRunTimeSelectionTable
(
    UnsortedMeshedSurface,
    GTSsurfaceFormat,
    triFace,
    fileExtension,
    gts
);
addNamedTemplatedToRunTimeSelectionTable
(
    UnsortedMeshedSurface,
    GTSsurfaceFormat,
    labelledTri,
    fileExtension,
    gts
);

// write MeshedSurface
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurface,
    GTSsurfaceFormat,
    face,
    write,
    fileExtension,
    gts
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurface,
    GTSsurfaceFormat,
    triFace,
    write,
    fileExtension,
    gts
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurface,
    GTSsurfaceFormat,
    labelledTri,
    write,
    fileExtension,
    gts
);

// write UnsortedMeshedSurface
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    GTSsurfaceFormat,
    face,
    write,
    fileExtension,
    gts
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    GTSsurfaceFormat,
    triFace,
    write,
    fileExtension,
    gts
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    GTSsurfaceFormat,
    labelledTri,
    write,
    fileExtension,
    gts
);

}
}

// ************************************************************************* //
