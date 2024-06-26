/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2017 OpenFOAM Foundation
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
    Foam::LiftForce

Group
    grpLagrangianIntermediateForceSubModels

Description
    Base class for particle lift force models

SourceFiles
    LiftForceI.H
    LiftForce.C

\*---------------------------------------------------------------------------*/

#ifndef LiftForce_H
#define LiftForce_H

#include "submodels/Kinematic/ParticleForces/ParticleForce/ParticleForce.H"
#include "fields/volFields/volFields.H"
#include "interpolation/interpolation/interpolation/interpolation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class LiftForce Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class LiftForce
:
    public ParticleForce<CloudType>
{
protected:

    // Protected data

        //- Name of velocity field
        const word UName_;

        //- Curl of carrier phase velocity interpolator
        autoPtr<interpolation<vector>> curlUcInterpPtr_;


    // Protected Member Functions

        //- Calculate the lift coefficient
        virtual scalar Cl
        (
            const typename CloudType::parcelType& p,
            const typename CloudType::parcelType::trackingData& td,
            const vector& curlUc,
            const scalar Re,
            const scalar muc
        ) const;


public:

    // Constructors

        //- Construct from mesh
        LiftForce
        (
            CloudType& owner,
            const fvMesh& mesh,
            const dictionary& dict,
            const word& forceType
        );

        //- Construct copy
        LiftForce(const LiftForce& lf);

        //- Construct and return a clone
        virtual autoPtr<ParticleForce<CloudType>> clone() const
        {
            return autoPtr<ParticleForce<CloudType>>
            (
                new LiftForce<CloudType>(*this)
            );
        }


    //- Destructor
    virtual ~LiftForce();


    // Member Functions

        // Access

            //- Return the curl of the carrier phase velocity interpolator
            inline const interpolation<vector>& curlUcInterp() const;


        // Evaluation

            //- Cache fields
            virtual void cacheFields(const bool store);

            //- Calculate the non-coupled force
            virtual forceSuSp calcCoupled
            (
                const typename CloudType::parcelType& p,
                const typename CloudType::parcelType::trackingData& td,
                const scalar dt,
                const scalar mass,
                const scalar Re,
                const scalar muc
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "submodels/Kinematic/ParticleForces/Lift/LiftForce/LiftForceI.H"

#ifdef NoRepository
    #include "submodels/Kinematic/ParticleForces/Lift/LiftForce/LiftForce.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
