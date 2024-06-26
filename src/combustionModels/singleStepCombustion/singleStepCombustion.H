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
    Foam::combustionModels::singleStepCombustion

Group
    grpCombustionModels

Description
    Base class for combustion models using singleStepReactingMixture.

SourceFiles
    singleStepCombustion.C

\*---------------------------------------------------------------------------*/

#ifndef singleStepCombustion_H
#define singleStepCombustion_H

#include "mixtures/singleStepReactingMixture/singleStepReactingMixture.H"
#include "CombustionModelCaseDir/ThermoCombustion/ThermoCombustion.H"
#include "fvMatrices/fvScalarMatrix/fvScalarMatrix.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace combustionModels
{

/*---------------------------------------------------------------------------*\
                    Class singleStepCombustion Declaration
\*---------------------------------------------------------------------------*/

template<class ReactionThermo, class ThermoType>
class singleStepCombustion
:
    public ThermoCombustion<ReactionThermo>
{
    // Private Member Functions

        //- No copy construct
        singleStepCombustion(const singleStepCombustion&) = delete;

        //- No copy assignment
        void operator=(const singleStepCombustion&) = delete;


protected:

    // Protected data

        //- Pointer to singleStepReactingMixture mixture
        singleStepReactingMixture<ThermoType>* singleMixturePtr_;

        //- Fuel consumption rate
        volScalarField wFuel_;

        //- Semi-implicit (true) or explicit (false) treatment
        bool semiImplicit_;


public:

    // Constructors

        //- Construct from components
        singleStepCombustion
        (
            const word& modelType,
            ReactionThermo& thermo,
            const compressibleTurbulenceModel& turb,
            const word& combustionProperties
        );


    //- Destructor
    virtual ~singleStepCombustion();


    // Member Functions

        //- Fuel consumption rate matrix
        virtual tmp<fvScalarMatrix> R(volScalarField& Y) const;

        //- Heat release rate [kg/m/s3]
        virtual tmp<volScalarField> Qdot() const;

        //- Update properties from given dictionary
        virtual bool read();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace combustionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "singleStepCombustion/singleStepCombustion.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
