/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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
    Foam::powerSeriesReactionRate

Description
    Power series reaction rate.

SourceFiles
    powerSeriesReactionRateI.H

\*---------------------------------------------------------------------------*/

#ifndef powerSeriesReactionRate_H
#define powerSeriesReactionRate_H

#include "fields/Fields/scalarField/scalarField.H"
#include "db/typeInfo/typeInfo.H"
#include "containers/Lists/FixedList/FixedList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators

class powerSeriesReactionRate;

Ostream& operator<<(Ostream&, const powerSeriesReactionRate&);


/*---------------------------------------------------------------------------*\
                   Class powerSeriesReactionRate Declaration
\*---------------------------------------------------------------------------*/

class powerSeriesReactionRate
{
    // Private data

        scalar A_;
        scalar beta_;
        scalar Ta_;

        static const label nCoeff_ = 4;
        FixedList<scalar, nCoeff_> coeffs_;


public:

    // Constructors

        //- Construct from components
        inline powerSeriesReactionRate
        (
            const scalar A,
            const scalar beta,
            const scalar Ta,
            const FixedList<scalar, nCoeff_> coeffs
        );

        //- Construct from dictionary
        inline powerSeriesReactionRate
        (
            const speciesTable& species,
            const dictionary& dict
        );


    // Member Functions

        //- Return the type name
        static word type()
        {
            return "powerSeries";
        }

        inline scalar operator()
        (
            const scalar p,
            const scalar T,
            const scalarField& c
        ) const;

        //- Write to stream
        inline void write(Ostream& os) const;


    // Ostream Operator

        inline friend Ostream& operator<<
        (
            Ostream&,
            const powerSeriesReactionRate&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "reaction/reactionRate/powerSeries/powerSeriesReactionRateI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
