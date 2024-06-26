/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011 OpenFOAM Foundation
    Copyright (C) 2019 OpenCFD Ltd.
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
    Foam::attachPolyTopoChanger

Description
    This class is derived from polyMesh and serves as a tool for
    statically connecting pieces of a mesh by executing the mesh
    modifiers and cleaning the mesh.

    The idea is that a mesh can be built from pieces and put together
    using various mesh modifiers (mainly sliding interfaces) which are
    not needed during the run.  Therefore, once the mesh is assembled
    and mesh modification triggered, the newly created point, face and
    cell zones can be cleared together with the mesh modifiers thus
    creating a singly connected static mesh.

    Note:
    All point, face and cell zoning will be lost!  Do it after
    attaching the parts of the mesh, as the point, face and cell
    numbering changes.

\*---------------------------------------------------------------------------*/

#ifndef Foam_attachPolyTopoChanger_H
#define Foam_attachPolyTopoChanger_H

#include "polyTopoChange/polyTopoChanger/polyTopoChanger.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                    Class attachPolyTopoChanger Declaration
\*---------------------------------------------------------------------------*/

class attachPolyTopoChanger
:
    public polyTopoChanger
{
public:

    // Generated Methods

        //- No copy construct
        attachPolyTopoChanger(const attachPolyTopoChanger&) = delete;

        //- No copy assignment
        void operator=(const attachPolyTopoChanger&) = delete;


    // Constructors

        //- Read constructor from IOobject and a polyMesh
        attachPolyTopoChanger(const IOobject& io, polyMesh&);

        //- Read constructor for given polyMesh
        explicit attachPolyTopoChanger(polyMesh&);


    //- Destructor
    virtual ~attachPolyTopoChanger() = default;


    // Member Functions

        //- Attach mesh. By default filter out empty patches.
        void attach(const bool removeEmptyPatches = true);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
