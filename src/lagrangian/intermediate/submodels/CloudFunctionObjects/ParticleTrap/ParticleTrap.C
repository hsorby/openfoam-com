/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2017 OpenFOAM Foundation
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

#include "submodels/CloudFunctionObjects/ParticleTrap/ParticleTrap.H"
#include "finiteVolume/fvc/fvcGrad.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ParticleTrap<CloudType>::ParticleTrap
(
    const dictionary& dict,
    CloudType& owner,
    const word& modelName
)
:
    CloudFunctionObject<CloudType>(dict, owner, modelName, typeName),
    alphaName_
    (
        this->coeffDict().template getOrDefault<word>("alpha", "alpha")
    ),
    alphaPtr_(nullptr),
    gradAlphaPtr_(nullptr),
    threshold_(this->coeffDict().getScalar("threshold"))
{}


template<class CloudType>
Foam::ParticleTrap<CloudType>::ParticleTrap
(
    const ParticleTrap<CloudType>& pt
)
:
    CloudFunctionObject<CloudType>(pt),
    alphaName_(pt.alphaName_),
    alphaPtr_(pt.alphaPtr_),
    gradAlphaPtr_(nullptr),
    threshold_(pt.threshold_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void Foam::ParticleTrap<CloudType>::preEvolve
(
    const typename parcelType::trackingData& td
)
{
    if (alphaPtr_ == nullptr)
    {
        const fvMesh& mesh = this->owner().mesh();
        const volScalarField& alpha =
            mesh.lookupObject<volScalarField>(alphaName_);

        alphaPtr_ = &alpha;
    }

    if (gradAlphaPtr_)
    {
        *gradAlphaPtr_ == fvc::grad(*alphaPtr_);
    }
    else
    {
        gradAlphaPtr_.reset(new volVectorField(fvc::grad(*alphaPtr_)));
    }
}


template<class CloudType>
void Foam::ParticleTrap<CloudType>::postEvolve
(
    const typename parcelType::trackingData& td
)
{
    gradAlphaPtr_.clear();
}


template<class CloudType>
bool Foam::ParticleTrap<CloudType>::postMove
(
    parcelType& p,
    const scalar,
    const point&,
    const typename parcelType::trackingData& td
)
{
    if (alphaPtr_->primitiveField()[p.cell()] < threshold_)
    {
        const vector& gradAlpha = gradAlphaPtr_()[p.cell()];
        vector nHat = gradAlpha/mag(gradAlpha);
        scalar nHatU = nHat & p.U();

        if (nHatU < 0)
        {
            p.U() -= 2*nHat*nHatU;
        }
    }

    return true;
}


// ************************************************************************* //
