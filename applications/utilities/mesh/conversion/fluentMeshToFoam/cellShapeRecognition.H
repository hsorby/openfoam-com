/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011 OpenFOAM Foundation
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

InClass
    Foam::cellShapeRecognition

Description

\*---------------------------------------------------------------------------*/

#ifndef cellShapeRecognition_H
#define cellShapeRecognition_H

#include "meshes/meshShapes/cellShape/cellShape.H"
#include "meshes/meshShapes/face/faceList.H"
#include "containers/PtrLists/PtrList/PtrList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

cellShape extrudedTriangleCellShape
(
    const label cellIndex,
    const labelList& faceLabels,
    const faceList& faces,
    const labelList& owner,
    const labelList& neighbour,
    const label pointOffset,
    faceList& frontAndBackFaces
);


cellShape extrudedQuadCellShape
(
    const label cellIndex,
    const labelList& faceLabels,
    const faceList& faces,
    const labelList& owner,
    const labelList& neighbour,
    const label pointOffset,
    faceList& frontAndBackFaces
);


cellShape create3DCellShape
(
    const label cellIndex,
    const labelList& faceLabels,
    const faceList& faces,
    const labelList& owner,
    const labelList& neighbour,
    const label fluentCellModelID
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
