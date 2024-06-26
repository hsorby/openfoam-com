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
    Foam::tetherPotential

Description

SourceFiles
    tetherPotential.C
    tetherPotentialNew.C

\*---------------------------------------------------------------------------*/

#ifndef tetherPotential_H
#define tetherPotential_H

#include "db/IOobjects/IOdictionary/IOdictionary.H"
#include "db/typeInfo/typeInfo.H"
#include "db/runTimeSelection/construction/runTimeSelectionTables.H"
#include "memory/autoPtr/autoPtr.H"
#include "primitives/Vector/floats/vector.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class tetherPotential Declaration
\*---------------------------------------------------------------------------*/

class tetherPotential
{

protected:

    // Protected data

        word name_;
        dictionary tetherPotentialProperties_;


    // Private Member Functions

        //- No copy construct
        tetherPotential(const tetherPotential&) = delete;

        //- No copy assignment
        void operator=(const tetherPotential&) = delete;


public:

    //- Runtime type information
    TypeName("tetherPotential");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            tetherPotential,
            dictionary,
            (
                const word& name,
                const dictionary& tetherPotentialProperties
            ),
            (name, tetherPotentialProperties)
        );


    // Selectors

        //- Return a reference to the selected viscosity model
        static autoPtr<tetherPotential> New
        (
            const word& name,
            const dictionary& tetherPotentialProperties
        );


    // Constructors

        //- Construct from components
        tetherPotential
        (
            const word& name,
            const dictionary& tetherPotentialProperties
        );


    //- Destructor
    virtual ~tetherPotential() = default;


    // Member Functions

        virtual scalar energy (const vector r) const = 0;

        virtual vector force (const vector r) const = 0;

        const dictionary& tetherPotentialProperties() const
        {
            return tetherPotentialProperties_;
        }

        //- Read tetherPotential dictionary
        virtual bool read(const dictionary& tetherPotentialProperties) = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
