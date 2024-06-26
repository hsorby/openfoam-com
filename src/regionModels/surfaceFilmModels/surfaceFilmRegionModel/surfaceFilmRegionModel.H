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
    Foam::regionModels::surfaceFilmModels::surfaceFilmRegionModel

Description
    Base class for surface film models

SourceFiles
    surfaceFilmRegionModel.C

\*---------------------------------------------------------------------------*/

#ifndef surfaceFilmRegionModel_H
#define surfaceFilmRegionModel_H

#include "surfaceFilmModel/surfaceFilmModel.H"
#include "singleLayerRegion/singleLayerRegion.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                      Class surfaceFilmRegionModel Declaration
\*---------------------------------------------------------------------------*/

class surfaceFilmRegionModel
:
    public surfaceFilmModel,
    public singleLayerRegion
{
    // Private Member Functions

        //- No copy construct
        surfaceFilmRegionModel(const surfaceFilmRegionModel&) = delete;

        //- No copy assignment
        void operator=(const surfaceFilmRegionModel&) = delete;


protected:

    // Protected data

        //- Acceleration due to gravity [m/s2]
        const dimensionedVector& g_;


    // Protected member functions

        //- Read control parameters from dictionary
        virtual bool read();


public:

    //- Runtime type information
    TypeName("surfaceFilmRegionModel");


    // Constructors

        //- Construct from type name, mesh and gravity vector
        surfaceFilmRegionModel
        (
            const word& modelType,
            const fvMesh& mesh,
            const dimensionedVector& g,
            const word& regionType
        );


    //- Destructor
    virtual ~surfaceFilmRegionModel();


    // Member Functions

        // Access

            //- Return the acceleration due to gravity
            inline const dimensionedVector& g() const;

            //- External hook to add sources to the film
            virtual void addSources
            (
                const label patchi,
                const label facei,
                const scalar massSource,
                const vector& momentumSource,
                const scalar pressureSource,
                const scalar energySource
            ) = 0;


        // Fields

            //- Return the film thickness [m]
            virtual const volScalarField& delta() const = 0;

            //- Return the film coverage, 1 = covered, 0 = uncovered / []
            virtual const volScalarField& alpha() const = 0;

            //- Return the film velocity [m/s]
            virtual const volVectorField& U() const = 0;

            //- Return the film surface velocity [m/s]
            virtual const volVectorField& Us() const = 0;

            //- Return the film wall velocity [m/s]
            virtual const volVectorField& Uw() const = 0;

            //- Return the film density [kg/m3]
            virtual const volScalarField& rho() const = 0;

            //- Return the film mean temperature [K]
            virtual const volScalarField& T() const = 0;

            //- Return the film surface temperature [K]
            virtual const volScalarField& Ts() const = 0;

            //- Return the film wall temperature [K]
            virtual const volScalarField& Tw() const = 0;

            //- Return the film surface temperature [J/kg]
            virtual const volScalarField& hs() const = 0;

            //- Return the film specific heat capacity [J/kg/K]
            virtual const volScalarField& Cp() const = 0;

            //- Return the film thermal conductivity [W/m/K]
            virtual const volScalarField& kappa() const = 0;

            //- Return the film surface tension [N/m]
            virtual const volScalarField& sigma() const = 0;


            // Transfer fields - to the primary region

                //- Return mass transfer source - Eulerian phase only
                virtual tmp<volScalarField> primaryMassTrans() const = 0;

                //- Return the film mass available for transfer
                virtual const volScalarField& cloudMassTrans() const = 0;

                //- Return the parcel diameters originating from film
                virtual const volScalarField& cloudDiameterTrans() const = 0;


        // Evolution

            //- Main driver routing to evolve the region - calls other evolves
            virtual void evolve();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "surfaceFilmRegionModel/surfaceFilmRegionModelI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
