/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2019-2021 OpenCFD Ltd.
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
    Foam::ReactionList

Description
    List of templated reactions

SourceFiles
    ReactionList.C

\*---------------------------------------------------------------------------*/

#ifndef ReactionList_H
#define ReactionList_H

#include "containers/PtrLists/PtrList/PtrList.H"
#include "containers/LinkedLists/user/SLPtrList.H"
#include "speciesTable/speciesTable.H"
#include "primitives/strings/fileName/fileName.H"
#include "reaction/ReactionsCaseDir/Reaction/Reaction.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class ReactionList Declaration
\*---------------------------------------------------------------------------*/

template<class ThermoType>
class ReactionList
:
    public SLPtrList<Reaction<ThermoType>>
{
    // Private Data

        //- Reference to the table of species
        const speciesTable& species_;

        //- Reference to the thermo database
        const ReactionTable<ThermoType>& thermoDb_;

        //- The dictionary used for construction
        const dictionary dict_;


    // Private Member Functions

        //- No copy assignment
        void operator=(const ReactionList&) = delete;


public:

    // Constructors

        //- Construct empty list
        ReactionList
        (
            const speciesTable& species,
            const ReactionTable<ThermoType>& thermoDatabase
        );

        //- Construct from dictionary
        ReactionList
        (
            const speciesTable& species,
            const ReactionTable<ThermoType>& thermoDatabase,
            const dictionary& dict
        );

        //- Construct copy
        ReactionList(const ReactionList<ThermoType>& reactions);


    //- Destructor
    ~ReactionList() = default;


    // Public Member Functions

        //- Read reactions from dictionary
        bool readReactionDict();

        //- Write
        void write(Ostream& os) const;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "reaction/ReactionsCaseDir/ReactionList/ReactionList.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
