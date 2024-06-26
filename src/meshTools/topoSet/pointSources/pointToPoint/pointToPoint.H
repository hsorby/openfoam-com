/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011 OpenFOAM Foundation
    Copyright (C) 2018-2020 OpenCFD Ltd.
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
    Foam::pointToPoint

Description
    A \c topoSetPointSource to select all points from given \c pointSet(s).

    Operands
    \table
      Operand   | Type        | Location
      input     | pointSet(s) | $FOAM_CASE/constant/polyMesh/sets/\<set\>
      output    | pointSet    | $FOAM_CASE/constant/polyMesh/sets/\<set\>
    \endtable

Usage
    Minimal example by using \c system/topoSetDict.actions:
    \verbatim
    {
        // Mandatory (inherited) entries
        name        <name>;
        type        pointSet;
        action      <action>;

        // Mandatory entries
        source      pointToPoint;

        // Conditional mandatory entries
        // Select either of the below

        // Option-1
        sets
        (
            <pointSetName1>
            <pointSetName2>
            ...
        );

        // Option-2
        set   <pointSetName>;
    }
    \endverbatim

    where the entries mean:
    \table
      Property   | Description                          | Type | Req'd | Dflt
      name       | Name of pointSet                     | word |  yes  | -
      type       | Type name: pointSet                  | word |  yes  | -
      action     | Action applied on points - see below | word |  yes  | -
      source     | Source name: pointToPoint            | word |  yes  | -
    \endtable

    Options for the \c action entry:
    \verbatim
      new      | Create a new pointSet from selected points
      add      | Add selected points into this pointSet
      subtract | Remove selected points from this pointSet
    \endverbatim

    Options for the conditional mandatory entries:
    \verbatim
      Entry    | Description                | Type     | Req'd  | Dflt
      sets     | Names of input pointSets   | wordList | cond'l | -
      set      | Name of input pointSet     | word     | cond'l | -
    \endverbatim

Note
    The order of precedence among the conditional mandatory entries from the
    highest to the lowest is \c sets, and \c set.

See also
    - Foam::topoSetSource
    - Foam::topoSetPointSource

SourceFiles
    pointToPoint.C

\*---------------------------------------------------------------------------*/

#ifndef pointToPoint_H
#define pointToPoint_H

#include "topoSet/pointSources/topoSetPointSource/topoSetPointSource.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class pointToPoint Declaration
\*---------------------------------------------------------------------------*/

class pointToPoint
:
    public topoSetPointSource
{
    // Private Data

        //- Add usage string
        static addToUsageTable usage_;

        //- Names of sets to use
        wordList names_;


public:

    //- Runtime type information
    TypeName("pointToPoint");


    // Constructors

        //- Construct from components
        pointToPoint(const polyMesh& mesh, const word& setName);

        //- Construct from dictionary
        pointToPoint(const polyMesh& mesh, const dictionary& dict);

        //- Construct from Istream
        pointToPoint(const polyMesh& mesh, Istream& is);


    //- Destructor
    virtual ~pointToPoint() = default;


    // Member Functions

        virtual void applyToSet
        (
            const topoSetSource::setAction action,
            topoSet& set
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
