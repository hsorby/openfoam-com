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

Class
    Foam::potential

Description

SourceFiles
    potentialI.H
    potential.C

\*---------------------------------------------------------------------------*/

#ifndef potential_H
#define potential_H

#include "meshes/polyMesh/polyMesh.H"
#include "db/IOobjects/IOdictionary/IOdictionary.H"
#include "db/Time/TimeOpenFOAM.H"
#include "pairPotential/pairPotentialList/pairPotentialList.H"
#include "electrostaticPotential/electrostaticPotential.H"
#include "tetherPotential/tetherPotentialList/tetherPotentialList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         Class potential Declaration
\*---------------------------------------------------------------------------*/

class potential
{
    // Private data

        const polyMesh& mesh_;

        List<word> idList_;

        List<word> siteIdList_;

        label nPairPotIds_;

        scalar potentialEnergyLimit_;

        labelList removalOrder_;

        pairPotentialList pairPotentials_;

        tetherPotentialList tetherPotentials_;

        vector gravity_;


    // Private Member Functions

        void setSiteIdList(const dictionary& moleculePropertiesDict);

        void readPotentialDict();

        void readMdInitialiseDict
        (
            const IOdictionary& mdInitialiseDict,
            IOdictionary& idListDict
        );

        //- No copy construct
        potential(const potential&) = delete;

        //- No copy assignment
        void operator=(const potential&) = delete;


public:

    // Constructors

        //- Construct from mesh reference
        potential(const polyMesh& mesh);

        //- Construct from mdInitialiseDict
        potential
        (
            const polyMesh& mesh,
            const IOdictionary& mdInitialiseDict,
            IOdictionary& idListDict
        );


    //- Destructor
    ~potential();


    // Member Functions

        // Access

            inline label nIds() const;

            inline const List<word>& idList() const;

            inline const List<word>& siteIdList() const;

            inline scalar potentialEnergyLimit() const;

            inline label nPairPotentials() const;

            inline const labelList& removalOrder() const;

            inline const pairPotentialList& pairPotentials() const;

            inline const tetherPotentialList& tetherPotentials() const;

            inline const vector& gravity() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "potential/potentialI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
