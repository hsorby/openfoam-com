/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2014 OpenFOAM Foundation
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
    Foam::compressibleTransportModel

Description
    Base-class for all transport models used by the compressible turbulence
    models.

SourceFiles
    compressibleTransportModel.C

\*---------------------------------------------------------------------------*/

#ifndef compressibleTransportModel_H
#define compressibleTransportModel_H

#include "fields/Fields/primitiveFieldsFwd.H"
#include "fields/volFields/volFieldsFwd.H"
#include "fields/surfaceFields/surfaceFieldsFwd.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                Class compressibleTransportModel Declaration
\*---------------------------------------------------------------------------*/

class compressibleTransportModel
{
    // Private Member Functions

        //- No copy construct
        compressibleTransportModel(const compressibleTransportModel&) = delete;

        //- No copy assignment
        void operator=(const compressibleTransportModel&) = delete;


public:

    //- Runtime type information
    TypeName("compressibleTransportModel");


    // Constructors

        //- Construct from components
        compressibleTransportModel
        ();


    //- Destructor
    virtual ~compressibleTransportModel();


    // Member Functions

        //- Return the dynamic laminar viscosity
        virtual tmp<volScalarField> mu() const = 0;

        //- Return the dynamic laminar viscosity for patch
        virtual tmp<scalarField> mu(const label patchi) const = 0;

        //- Return the laminar viscosity
        virtual tmp<volScalarField> nu() const = 0;

        //- Return the laminar viscosity for patch
        virtual tmp<scalarField> nu(const label patchi) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
