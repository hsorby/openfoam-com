/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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
    Foam::setUpdater

Description
    Keeps cell/face/vertex sets uptodate. Both the ones loaded and the ones
    on disk.

SourceFiles
    setUpdater.C
    setUpdaterTemplates.C

\*---------------------------------------------------------------------------*/

#ifndef setUpdater_H
#define setUpdater_H

#include "polyTopoChange/polyMeshModifier/polyMeshModifier.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class setUpdater Declaration
\*---------------------------------------------------------------------------*/

class setUpdater
:
    public polyMeshModifier
{
    // Private Member Functions

        //- Update all sets of the given type
        template<class SetType>
        static void updateSets(const mapPolyMesh& map);

        //- No copy construct
        setUpdater(const setUpdater&) = delete;

        //- No copy assignment
        void operator=(const setUpdater&) = delete;


public:

    //- Runtime type information
    TypeName("setUpdater");


    // Constructors

        //- Construct from dictionary
        setUpdater
        (
            const word& name,
            const dictionary& dict,
            const label index,
            const polyTopoChanger& mme
        );


    //- Destructor
    virtual ~setUpdater() = default;


    // Member Functions

        //- Check for topology change
        virtual bool changeTopology() const;

        //- Insert the layer addition/removal instructions
        //  into the topological change
        virtual void setRefinement(polyTopoChange&) const;

        //- Modify motion points to comply with the topological change
        virtual void modifyMotionPoints(pointField& motionPoints) const;

        //- Force recalculation of locally stored data on topological change
        virtual void updateMesh(const mapPolyMesh&);

        //- Write
        virtual void write(Ostream&) const;

        //- Write dictionary
        virtual void writeDict(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
#ifdef NoRepository
    #include "setUpdater/setUpdaterTemplates.C"
#endif


#endif

// ************************************************************************* //
