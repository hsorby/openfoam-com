/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2017 OpenFOAM Foundation
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
    Foam::regionModels::surfaceFilmModel

Description
    Base class for surface film models

SourceFiles
    surfaceFilmModelI.H
    surfaceFilmModel.C
    surfaceFilmModelNew.C

\*---------------------------------------------------------------------------*/

#ifndef surfaceFilmModel_H
#define surfaceFilmModel_H

#include "db/runTimeSelection/construction/runTimeSelectionTables.H"
#include "fields/volFields/volFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{

/*---------------------------------------------------------------------------*\
                      Class surfaceFilmModel Declaration
\*---------------------------------------------------------------------------*/

class surfaceFilmModel
{
    // Private Member Functions

        //- No copy construct
        surfaceFilmModel(const surfaceFilmModel&) = delete;

        //- No copy assignment
        void operator=(const surfaceFilmModel&) = delete;


public:

    //- Runtime type information
    TypeName("surfaceFilmModel");

    //- Reference temperature for enthalpy
    static const dimensionedScalar Tref;


    // Declare runtime constructor selection table

         declareRunTimeSelectionTable
         (
             autoPtr,
             surfaceFilmModel,
             mesh,
             (
                const word& modelType,
                const fvMesh& mesh,
                const dimensionedVector& g,
                const word& regionType
             ),
             (modelType, mesh, g, regionType)
         );


    //- Constructor
    surfaceFilmModel();


    // Selectors

        //- Return a reference to the selected surface film model
        static autoPtr<surfaceFilmModel> New
        (
            const fvMesh& mesh,
            const dimensionedVector& g,
            const word& regionType = "surfaceFilm"
        );


    //- Destructor
    virtual ~surfaceFilmModel();


    // Member Functions

        // Solution parameters

            //- Courant number evaluation
            virtual scalar CourantNumber() const = 0;


        // Primary region source fields

            //- Return total mass source - Eulerian phase only
            virtual tmp<volScalarField::Internal> Srho() const = 0;

            //- Return mass source for specie i - Eulerian phase only
            virtual tmp<volScalarField::Internal> Srho
            (
                const label i
            ) const = 0;

            //- Return enthalpy source - Eulerian phase only
            virtual tmp<volScalarField::Internal> Sh() const = 0;


        // Evolution

            //- Main driver routing to evolve the region - calls other evolves
            virtual void evolve() = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
