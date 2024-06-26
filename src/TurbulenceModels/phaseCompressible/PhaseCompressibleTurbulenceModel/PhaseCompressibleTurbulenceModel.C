/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2013-2017 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "TurbulenceModels/phaseCompressible/PhaseCompressibleTurbulenceModel/PhaseCompressibleTurbulenceModelPascal.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class TransportModel>
Foam::PhaseCompressibleTurbulenceModel<TransportModel>::
PhaseCompressibleTurbulenceModel
(
    const word& type,
    const volScalarField& alpha,
    const volScalarField& rho,
    const volVectorField& U,
    const surfaceScalarField& alphaRhoPhi,
    const surfaceScalarField& phi,
    const transportModel& transport,
    const word& propertiesName
)
:
    TurbulenceModel
    <
        volScalarField,
        volScalarField,
        compressibleTurbulenceModel,
        transportModel
    >
    (
        alpha,
        rho,
        U,
        alphaRhoPhi,
        phi,
        transport,
        propertiesName
    )
{}


// * * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * //

template<class TransportModel>
Foam::autoPtr<Foam::PhaseCompressibleTurbulenceModel<TransportModel>>
Foam::PhaseCompressibleTurbulenceModel<TransportModel>::New
(
    const volScalarField& alpha,
    const volScalarField& rho,
    const volVectorField& U,
    const surfaceScalarField& alphaRhoPhi,
    const surfaceScalarField& phi,
    const transportModel& transport,
    const word& propertiesName
)
{
    return autoPtr<PhaseCompressibleTurbulenceModel>
    (
        static_cast<PhaseCompressibleTurbulenceModel*>(
        TurbulenceModel
        <
            volScalarField,
            volScalarField,
            compressibleTurbulenceModel,
            transportModel
        >::New
        (
            alpha,
            rho,
            U,
            alphaRhoPhi,
            phi,
            transport,
            propertiesName
        ).ptr())
    );
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class TransportModel>
Foam::tmp<Foam::volScalarField>
Foam::PhaseCompressibleTurbulenceModel<TransportModel>::pPrime() const
{
    return tmp<volScalarField>::New
    (
        IOobject
        (
            IOobject::groupName("pPrime", this->alphaRhoPhi_.group()),
            this->runTime_.timeName(),
            this->mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        this->mesh_,
        dimensionedScalar(dimPressure, Zero)
    );
}


template<class TransportModel>
Foam::tmp<Foam::surfaceScalarField>
Foam::PhaseCompressibleTurbulenceModel<TransportModel>::pPrimef() const
{
    return tmp<surfaceScalarField>::New
    (
        IOobject
        (
            IOobject::groupName("pPrimef", this->alphaRhoPhi_.group()),
            this->runTime_.timeName(),
            this->mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        this->mesh_,
        dimensionedScalar(dimPressure, Zero)
    );
}


// ************************************************************************* //
