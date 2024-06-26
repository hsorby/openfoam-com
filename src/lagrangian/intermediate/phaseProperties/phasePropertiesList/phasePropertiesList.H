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
    Foam::phasePropertiesList

Description
    Simple container for a list of phase properties

SourceFiles
    phasePropertiesList.C

\*---------------------------------------------------------------------------*/

#ifndef phasePropertiesList_H
#define phasePropertiesList_H

#include "db/IOstreams/IOstreams/Istream.H"
#include "fields/volFields/volFields.H"
#include "primitives/strings/lists/wordList.H"
#include "phaseProperties/phaseProperties/phaseProperties.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                    Class phasePropertiesList Declaration
\*---------------------------------------------------------------------------*/

class phasePropertiesList
{
    // Private data

        //- List of phase properties
        List<phaseProperties> props_;

        //- List of word representation of phase types
        wordList phaseTypeNames_;

        //- List of state labels
        wordList stateLabels_;


public:

     // Constructors

        //- Construct null
        phasePropertiesList();

        //- Construct from components
        phasePropertiesList
        (
            Istream& is,
            const wordList& gasNames,
            const wordList& liquidNames,
            const wordList& solidNames
        );


    //- Destructor
    ~phasePropertiesList();


    // Public Member Functions

        //- Return the list of phase properties
        const List<phaseProperties>& props() const;

        //- Return the list of word representation of phase types
        const wordList& phaseTypes() const;

        //- Return the list of state labels
        const wordList& stateLabels() const;

        //- Return the size (number of phases)
        label size() const;


    // Member operators

        const phaseProperties& operator[](const label) const;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
