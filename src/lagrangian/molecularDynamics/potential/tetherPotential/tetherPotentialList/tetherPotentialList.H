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
    Foam::tetherPotentialList

Description

SourceFiles
    tetherPotentialList.C

\*---------------------------------------------------------------------------*/

#ifndef tetherPotentialList_H
#define tetherPotentialList_H

#include "containers/Lists/ListOps/ListOps.H"
#include "containers/PtrLists/PtrList/PtrList.H"
#include "primitives/strings/word/word.H"
#include "tetherPotential/basic/tetherPotential.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class tetherPotentialList Declaration
\*---------------------------------------------------------------------------*/

class tetherPotentialList
:
    public PtrList<tetherPotential>
{
    // Private data

        List<label> idMap_;


    // Private Member Functions

        inline label tetherPotentialIndex
        (
            const label a
        ) const;

        void readTetherPotentialDict
        (
            const List<word>& siteIdList,
            const dictionary& tetherPotentialDict,
            const List<word>& tetherSiteIdList
        );

        //- No copy construct
        tetherPotentialList(const tetherPotentialList&) = delete;

        //- No copy assignment
        void operator=(const tetherPotentialList&) = delete;


public:

    // Constructors

        tetherPotentialList();

        //- Construct from siteIdList and potential dictionaries
        tetherPotentialList
        (
            const List<word>& siteIdList,
            const dictionary& tetherPotentialDict,
            const List<word>& tetherSiteIdList
        );


    //- Destructor
    ~tetherPotentialList();


    // Member Functions

        void buildPotentials
        (
            const List<word>& siteIdList,
            const dictionary& tetherPotentialDict,
            const List<word>& tetherSiteIdList
        );

        // Access

            inline const List<word>& idMap() const;

            const tetherPotential& tetherPotentialFunction(const label a) const;

            vector force(const label a, const vector rIT) const;

            scalar energy (const label a, const vector rIT) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "tetherPotential/tetherPotentialList/tetherPotentialListI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
