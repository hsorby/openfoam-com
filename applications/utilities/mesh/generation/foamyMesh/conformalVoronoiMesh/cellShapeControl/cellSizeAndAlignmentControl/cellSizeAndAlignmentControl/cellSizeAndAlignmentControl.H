/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2015 OpenFOAM Foundation
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
    Foam::cellSizeAndAlignmentControl

Description

SourceFiles
    cellSizeAndAlignmentControlI.H
    cellSizeAndAlignmentControl.C

\*---------------------------------------------------------------------------*/

#ifndef cellSizeAndAlignmentControl_H
#define cellSizeAndAlignmentControl_H

#include "db/dictionary/dictionary.H"
#include "conformationSurfaces/conformationSurfaces.H"
#include "db/Time/TimeOpenFOAM.H"
#include "primitives/quaternion/quaternion.H"
#include "fields/Fields/triadField/triadField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{


/*---------------------------------------------------------------------------*\
                Class cellSizeAndAlignmentControl Declaration
\*---------------------------------------------------------------------------*/

class cellSizeAndAlignmentControl
{
protected:

        const Time& runTime_;

        const scalar& defaultCellSize_;

        Switch forceInitialPointInsertion_;


private:

    // Private data

        const word name_;


    // Private Member Functions

        //- No copy construct
        cellSizeAndAlignmentControl
        (
            const cellSizeAndAlignmentControl&
        ) = delete;

        //- No copy assignment
        void operator=(const cellSizeAndAlignmentControl&) = delete;


public:

    //- Runtime type information
    TypeName("cellSizeAndAlignmentControl");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            cellSizeAndAlignmentControl,
            dictionary,
            (
                const Time& runTime,
                const word& name,
                const dictionary& controlFunctionDict,
                const conformationSurfaces& geometryToConformTo,
                const scalar& defaultCellSize
            ),
            (
                runTime,
                name,
                controlFunctionDict,
                geometryToConformTo,
                defaultCellSize
            )
        );


    // Constructors

        //- Construct from dictionary and references to conformalVoronoiMesh and
        //  searchableSurfaces
        cellSizeAndAlignmentControl
        (
            const Time& runTime,
            const word& name,
            const dictionary& controlFunctionDict,
            const conformationSurfaces& geometryToConformTo,
            const scalar& defaultCellSize
        );


    // Selectors

        //- Return a reference to the selected cellShapeControl
        static autoPtr<cellSizeAndAlignmentControl> New
        (
            const Time& runTime,
            const word& name,
            const dictionary& controlFunctionDict,
            const conformationSurfaces& geometryToConformTo,
            const scalar& defaultCellSize
        );


    //- Destructor
    virtual ~cellSizeAndAlignmentControl() = default;


    // Member Functions

        // Access

            const word& name() const
            {
                return name_;
            }

            Switch forceInitialPointInsertion() const noexcept
            {
                return forceInitialPointInsertion_;
            }


        // Query

            virtual label maxPriority() const = 0;

            virtual void cellSizeFunctionVertices
            (
                DynamicList<Foam::point>& pts,
                DynamicList<scalar>& sizes
            ) const = 0;

            virtual void initialVertices
            (
                pointField& pts,
                scalarField& sizes,
                triadField& alignments
            ) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
