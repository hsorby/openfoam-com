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

Namespace
    Foam::compressibilityModels

Description
    Namespace for compressibility models.


Class
    Foam::barotropicCompressibilityModel

Description
    Abstract class for barotropic compressibility models

SourceFiles
    barotropicCompressibilityModel.C
    newbarotropicCompressibilityModel.C

\*---------------------------------------------------------------------------*/

#ifndef barotropicCompressibilityModel_H
#define barotropicCompressibilityModel_H

#include "db/IOobjects/IOdictionary/IOdictionary.H"
#include "db/typeInfo/typeInfo.H"
#include "db/runTimeSelection/construction/runTimeSelectionTables.H"
#include "fields/volFields/volFields.H"
#include "dimensionedTypes/dimensionedScalar/dimensionedScalar.H"
#include "memory/autoPtr/autoPtr.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                Class barotropicCompressibilityModel Declaration
\*---------------------------------------------------------------------------*/

class barotropicCompressibilityModel
{

protected:

    // Protected data

        dictionary compressibilityProperties_;

        volScalarField psi_;
        const volScalarField& gamma_;


    // Private Member Functions

        //- No copy construct
        barotropicCompressibilityModel
        (
            const barotropicCompressibilityModel&
        ) = delete;

        //- No copy assignment
        void operator=(const barotropicCompressibilityModel&) = delete;


public:

    //- Runtime type information
    TypeName("barotropicCompressibilityModel");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            barotropicCompressibilityModel,
            dictionary,
            (
                const dictionary& compressibilityProperties,
                const volScalarField& gamma,
                const word& psiName
            ),
            (compressibilityProperties, gamma, psiName)
        );


    // Selectors

        //- Return a reference to the selected compressibility model
        static autoPtr<barotropicCompressibilityModel> New
        (
            const dictionary& compressibilityProperties,
            const volScalarField& gamma,
            const word& psiName = "psi"
        );


    // Constructors

        //- Construct from components
        barotropicCompressibilityModel
        (
            const dictionary& compressibilityProperties,
            const volScalarField& gamma,
            const word& psiName = "psi"
        );


    //- Destructor
    virtual ~barotropicCompressibilityModel() = default;


    // Member Functions

        //- Return the phase transport properties dictionary
        const dictionary& compressibilityProperties() const
        {
            return compressibilityProperties_;
        }

        //- Return the compressibility
        const volScalarField& psi() const
        {
            return psi_;
        }

        //- Correct the compressibility
        virtual void correct() = 0;

        //- Read compressibilityProperties dictionary
        virtual bool read(const dictionary& compressibilityProperties) = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
