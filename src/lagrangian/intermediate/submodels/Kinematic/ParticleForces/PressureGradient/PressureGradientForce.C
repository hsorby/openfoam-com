/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2020 OpenCFD Ltd.
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

#include "submodels/Kinematic/ParticleForces/PressureGradient/PressureGradientForce.H"
#include "finiteVolume/fvc/fvcDdt.H"
#include "finiteVolume/fvc/fvcGrad.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::PressureGradientForce<CloudType>::PressureGradientForce
(
    CloudType& owner,
    const fvMesh& mesh,
    const dictionary& dict,
    const word& forceType
)
:
    ParticleForce<CloudType>(owner, mesh, dict, forceType, true),
    UName_(this->coeffs().template getOrDefault<word>("U", "U")),
    DUcDtInterpPtr_(nullptr)
{}


template<class CloudType>
Foam::PressureGradientForce<CloudType>::PressureGradientForce
(
    const PressureGradientForce& pgf
)
:
    ParticleForce<CloudType>(pgf),
    UName_(pgf.UName_),
    DUcDtInterpPtr_(nullptr)
{}


// * * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::PressureGradientForce<CloudType>::~PressureGradientForce()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void Foam::PressureGradientForce<CloudType>::cacheFields(const bool store)
{
    static word resultName("DUcDt");

    volVectorField* resultPtr =
        this->mesh().template getObjectPtr<volVectorField>(resultName);

    if (store)
    {
        if (!resultPtr)
        {
            const volVectorField& Uc = this->mesh().template
                lookupObject<volVectorField>(UName_);

            resultPtr = new volVectorField
            (
                resultName,
                fvc::ddt(Uc) + (Uc & fvc::grad(Uc))
            );

            resultPtr->store();
        }
        const volVectorField& DUcDt = *resultPtr;

        DUcDtInterpPtr_.reset
        (
            interpolation<vector>::New
            (
                this->owner().solution().interpolationSchemes(),
                DUcDt
            ).ptr()
        );
    }
    else
    {
        DUcDtInterpPtr_.clear();

        if (resultPtr)
        {
            resultPtr->checkOut();
        }
    }
}


template<class CloudType>
Foam::forceSuSp Foam::PressureGradientForce<CloudType>::calcCoupled
(
    const typename CloudType::parcelType& p,
    const typename CloudType::parcelType::trackingData& td,
    const scalar dt,
    const scalar mass,
    const scalar Re,
    const scalar muc
) const
{
    forceSuSp value(Zero);

    vector DUcDt =
        DUcDtInterp().interpolate(p.coordinates(), p.currentTetIndices());

    value.Su() = mass*td.rhoc()/p.rho()*DUcDt;

    return value;
}


template<class CloudType>
Foam::scalar Foam::PressureGradientForce<CloudType>::massAdd
(
    const typename CloudType::parcelType&,
    const typename CloudType::parcelType::trackingData& td,
    const scalar
) const
{
    return 0.0;
}


// ************************************************************************* //
