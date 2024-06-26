/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2019-2020 OpenCFD Ltd.
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

#include "submodels/ReactingMultiphase/InjectionModel/ReactingMultiphaseLookupTableInjection/ReactingMultiphaseLookupTableInjection.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ReactingMultiphaseLookupTableInjection<CloudType>::
ReactingMultiphaseLookupTableInjection
(
    const dictionary& dict,
    CloudType& owner,
    const word& modelName
)
:
    InjectionModel<CloudType>(dict, owner, modelName, typeName),
    inputFileName_(this->coeffDict().lookup("inputFile")),
    duration_(this->coeffDict().getScalar("duration")),
    parcelsPerSecond_(this->coeffDict().getScalar("parcelsPerSecond")),
    randomise_(this->coeffDict().getBool("randomise")),
    injectors_
    (
        IOobject
        (
            inputFileName_,
            owner.db().time().constant(),
            owner.db(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    ),
    injectorCells_(injectors_.size()),
    injectorTetFaces_(injectors_.size()),
    injectorTetPts_(injectors_.size())
{
    updateMesh();

    duration_ = owner.db().time().userTimeToTime(duration_);

    // Determine volume of particles to inject
    this->volumeTotal_ = 0.0;
    forAll(injectors_, i)
    {
        this->volumeTotal_ += injectors_[i].mDot()/injectors_[i].rho();
    }
    this->volumeTotal_ *= duration_;
}


template<class CloudType>
Foam::ReactingMultiphaseLookupTableInjection<CloudType>::
ReactingMultiphaseLookupTableInjection
(
    const ReactingMultiphaseLookupTableInjection<CloudType>& im
)
:
    InjectionModel<CloudType>(im),
    inputFileName_(im.inputFileName_),
    duration_(im.duration_),
    parcelsPerSecond_(im.parcelsPerSecond_),
    randomise_(im.randomise_),
    injectors_(im.injectors_),
    injectorCells_(im.injectorCells_),
    injectorTetFaces_(im.injectorTetFaces_),
    injectorTetPts_(im.injectorTetPts_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void Foam::ReactingMultiphaseLookupTableInjection<CloudType>::updateMesh()
{
    // Set/cache the injector cells
    bitSet reject(injectors_.size());

    // Set/cache the injector cells
    forAll(injectors_, i)
    {
        if
        (
            !this->findCellAtPosition
            (
                injectorCells_[i],
                injectorTetFaces_[i],
                injectorTetPts_[i],
                injectors_[i].x(),
                !this->ignoreOutOfBounds_

            )
        )
        {
            reject.set(i);
        }
    }

    label nRejected = reject.count();

    if (nRejected)
    {
        reject.flip();
        inplaceSubset(reject, injectorCells_);
        inplaceSubset(reject, injectorTetFaces_);
        inplaceSubset(reject, injectorTetPts_);
        inplaceSubset(reject, injectors_);

        Info<< "    " << nRejected
            << " positions rejected, out of bounds" << endl;
    }
}


template<class CloudType>
Foam::scalar
Foam::ReactingMultiphaseLookupTableInjection<CloudType>::timeEnd() const
{
    return this->SOI_ + duration_;
}


template<class CloudType>
Foam::label
Foam::ReactingMultiphaseLookupTableInjection<CloudType>::parcelsToInject
(
    const scalar time0,
    const scalar time1
)
{
    if ((time0 >= 0.0) && (time0 < duration_))
    {
        return floor(injectorCells_.size()*(time1 - time0)*parcelsPerSecond_);
    }

    return 0;
}


template<class CloudType>
Foam::scalar
Foam::ReactingMultiphaseLookupTableInjection<CloudType>::volumeToInject
(
    const scalar time0,
    const scalar time1
)
{
    scalar volume = 0.0;
    if ((time0 >= 0.0) && (time0 < duration_))
    {
        forAll(injectors_, i)
        {
            volume += injectors_[i].mDot()/injectors_[i].rho()*(time1 - time0);
        }
    }

    return volume;
}


template<class CloudType>
void Foam::ReactingMultiphaseLookupTableInjection<CloudType>::setPositionAndCell
(
    const label parcelI,
    const label nParcels,
    const scalar time,
    vector& position,
    label& cellOwner,
    label& tetFacei,
    label& tetPti
)
{
    label injectorI = 0;
    if (randomise_)
    {
        Random& rnd = this->owner().rndGen();
        injectorI = rnd.position<label>(0, injectorCells_.size() - 1);
    }
    else
    {
        injectorI = parcelI*injectorCells_.size()/nParcels;
    }

    position = injectors_[injectorI].x();
    cellOwner = injectorCells_[injectorI];
    tetFacei = injectorTetFaces_[injectorI];
    tetPti = injectorTetPts_[injectorI];
}


template<class CloudType>
void Foam::ReactingMultiphaseLookupTableInjection<CloudType>::setProperties
(
    const label parcelI,
    const label nParcels,
    const scalar,
    typename CloudType::parcelType& parcel
)
{
    label injectorI = parcelI*injectorCells_.size()/nParcels;

    // set particle velocity
    parcel.U() = injectors_[injectorI].U();

    // set particle diameter
    parcel.d() = injectors_[injectorI].d();

    // set particle density
    parcel.rho() = injectors_[injectorI].rho();

    // set particle temperature
    parcel.T() = injectors_[injectorI].T();

    // set particle specific heat capacity
    parcel.Cp() = injectors_[injectorI].Cp();

    // set particle component mass fractions
    parcel.Y() = injectors_[injectorI].Y();

    // set particle gaseous component mass fractions
    parcel.YGas() = injectors_[injectorI].YGas();

    // set particle liquid component mass fractions
    parcel.YLiquid() = injectors_[injectorI].YLiquid();

    // set particle solid component mass fractions
    parcel.YSolid() = injectors_[injectorI].YSolid();
}


template<class CloudType>
bool
Foam::ReactingMultiphaseLookupTableInjection<CloudType>::fullyDescribed() const
{
    return true;
}


template<class CloudType>
bool Foam::ReactingMultiphaseLookupTableInjection<CloudType>::validInjection
(
    const label
)
{
    return true;
}


// ************************************************************************* //
