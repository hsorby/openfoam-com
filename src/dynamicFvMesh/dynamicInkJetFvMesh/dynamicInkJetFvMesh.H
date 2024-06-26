/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011 OpenFOAM Foundation
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

Class
    Foam::dynamicInkJetFvMesh

Description
    Mesh motion specifically for the "pumping" system of an ink-jet
    injector.

    The set of points in the "pumping" region are compressed and expanded
    sinusoidally to impose a sinusoidal variation of the flow at the
    nozzle exit.

SourceFiles
    dynamicInkJetFvMesh.C

\*---------------------------------------------------------------------------*/

#ifndef dynamicInkJetFvMesh_H
#define dynamicInkJetFvMesh_H

#include "dynamicFvMesh/dynamicFvMesh.H"
#include "db/dictionary/dictionary.H"
#include "meshes/primitiveShapes/point/pointIOField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class dynamicInkJetFvMesh Declaration
\*---------------------------------------------------------------------------*/

class dynamicInkJetFvMesh
:
    public dynamicFvMesh
{
    // Private data

        dictionary dynamicMeshCoeffs_;

        scalar amplitude_;
        scalar frequency_;
        scalar refPlaneX_;

        pointIOField stationaryPoints_;


    // Private Member Functions

        //- No copy construct
        dynamicInkJetFvMesh(const dynamicInkJetFvMesh&) = delete;

        //- No copy assignment
        void operator=(const dynamicInkJetFvMesh&) = delete;


public:

    //- Runtime type information
    TypeName("dynamicInkJetFvMesh");


    // Constructors

        //- Construct from IOobject
        dynamicInkJetFvMesh(const IOobject& io, const bool doInit=true);


    //- Destructor
    ~dynamicInkJetFvMesh();


    // Member Functions

        //- Update the mesh for both mesh motion and topology change
        virtual bool update();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
