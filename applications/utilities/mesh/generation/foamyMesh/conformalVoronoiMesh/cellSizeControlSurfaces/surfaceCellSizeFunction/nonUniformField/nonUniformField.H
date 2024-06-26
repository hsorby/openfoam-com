/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2015 OpenFOAM Foundation
    Copyright (C) 2019-2020 OpenCFD Ltd.
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
    Foam::nonUniformField

Description

SourceFiles
    nonUniformField.C

\*---------------------------------------------------------------------------*/

#ifndef nonUniformField_H
#define nonUniformField_H

#include "triSurface/fields/triSurfaceFields.H"
#include "interpolations/primitivePatchInterpolation/primitivePatchInterpolation.H"
#include "cellSizeControlSurfaces/surfaceCellSizeFunction/surfaceCellSizeFunction/surfaceCellSizeFunction.H"
#include "cellSizeControlSurfaces/surfaceCellSizeFunction/cellSizeCalculationType/cellSizeCalculationType/cellSizeCalculationType.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class triSurfaceMesh;
class searchableSurface;

/*---------------------------------------------------------------------------*\
                           Class nonUniformField Declaration
\*---------------------------------------------------------------------------*/

class nonUniformField
:
    public surfaceCellSizeFunction
{

protected:

    // Typedefs

        typedef PrimitivePatchInterpolation
        <
            PrimitivePatch<::Foam::List<labelledTri>, pointField>
        >   primitivePatchInterpolation;


    // Private data

        const triSurfaceMesh& surfaceTriMesh_;

        autoPtr<cellSizeCalculationType> cellSizeCalculationType_;

        triSurfacePointScalarField pointCellSize_;


public:

    //- Runtime type information
    TypeName("nonUniformField");


    // Constructors

        //- Construct from components
        nonUniformField
        (
            const dictionary& cellSizeFunctionDict,
            const searchableSurface& surface,
            const scalar& defaultCellSize
        );


    //- Destructor
    virtual ~nonUniformField() = default;


    // Member Functions

        // Query

            //- Return the interpolated cell size for a point in the given
            //  surface triangle
            virtual scalar interpolate
            (
                const point& pt,
                const label index
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
