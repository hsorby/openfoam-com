/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2015 OpenFOAM Foundation
    Copyright (C) 2018 OpenCFD Ltd.
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

Class
    Foam::pointFile

Description
    Inserts points at locations specified in a pointFile into the surfaces to
    be conformed to of the conformalVoronoiMesh

SourceFiles
    pointFile.C

\*---------------------------------------------------------------------------*/

#ifndef pointFile_H
#define pointFile_H

#include "primitives/strings/fileName/fileName.H"
#include "meshes/primitiveShapes/point/pointIOField.H"
#include "primitives/bools/Switch/Switch.H"
#include "initialPointsMethod/initialPointsMethod/initialPointsMethod.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class pointFile Declaration
\*---------------------------------------------------------------------------*/

class pointFile
:
    public initialPointsMethod
{

private:

    // Private data

        //- The initial cell spacing
        fileName pointFileName_;

        //- Check if inserted points are inside or outside
        Switch insideOutsideCheck_;

        //- Should the initial positions be randomised
        Switch randomiseInitialGrid_;

        //- Randomise the initial positions by fraction of the initialCellSize_
        scalar randomPerturbationCoeff_;


public:

    //- Runtime type information
    TypeName("pointFile");

    // Constructors

        //- Construct from components
        pointFile
        (
            const dictionary& initialPointsDict,
            const Time& runTime,
            Random& rndGen,
            const conformationSurfaces& geometryToConformTo,
            const cellShapeControl& cellShapeControls,
            const autoPtr<backgroundMeshDecomposition>& decomposition
        );


    //- Destructor
    virtual ~pointFile() = default;


    // Member Functions

        //- Return the initial points for the conformalVoronoiMesh
        virtual List<Vb::Point> initialPoints() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
