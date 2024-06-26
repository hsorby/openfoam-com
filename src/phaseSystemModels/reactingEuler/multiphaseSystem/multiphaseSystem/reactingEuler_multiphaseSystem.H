/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2013-2018 OpenFOAM Foundation
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
    Foam::multiphaseSystem

Description
    Class which solves the volume fraction equations for two phases.

SourceFiles
    multiphaseSystem.C

\*---------------------------------------------------------------------------*/

#ifndef reactingEuler_multiphaseSystem_H
#define reactingEuler_multiphaseSystem_H

#include "phaseSystem/phaseSystem.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward Declarations
class dragModel;
class virtualMassModel;

/*---------------------------------------------------------------------------*\
                         Class multiphaseSystem Declaration
\*---------------------------------------------------------------------------*/

class multiphaseSystem
:
    public phaseSystem
{
    // Private Typedefs

        typedef HashTable<scalar, phasePairKey, phasePairKey::hash>
            cAlphaTable;


    // Private Data

        //- The indexed phase-fraction field;
        //  E.g., 1 for water, 2 for air, 3 for oil, etc...
        volScalarField alphas_;

        //-
        cAlphaTable cAlphas_;

        //- Stabilisation for normalisation of the interface normal
        const dimensionedScalar deltaN_;


    // Private Member Functions

        void calcAlphas();

        void solveAlphas();

        tmp<surfaceVectorField> nHatfv
        (
            const volScalarField& alpha1,
            const volScalarField& alpha2
        ) const;

        tmp<surfaceScalarField> nHatf
        (
            const volScalarField& alpha1,
            const volScalarField& alpha2
        ) const;

        void correctContactAngle
        (
            const phaseModel& alpha1,
            const phaseModel& alpha2,
            surfaceVectorField::Boundary& nHatb
        ) const;

        tmp<volScalarField> K
        (
            const phaseModel& alpha1,
            const phaseModel& alpha2
        ) const;

        //- Return the drag coefficient for phase pair
        virtual tmp<volScalarField> Kd(const phasePairKey& key) const = 0;

        //- Return the virtual mass coefficient for phase pair
        virtual tmp<volScalarField> Vm(const phasePairKey& key) const = 0;


protected:

    // Protected data

        //- Flag to indicate that returned lists of fields are "complete"; i.e.,
        //  that an absence of force is returned as a constructed list of zeros,
        //  rather than a null pointer
        static const bool fillFields_ = false;


public:

    //- Runtime type information
    TypeName("multiphaseSystem");

    // Declare runtime construction

        declareRunTimeSelectionTable
        (
            autoPtr,
            multiphaseSystem,
            dictionary,
            (
                const fvMesh& mesh
            ),
            (mesh)
        );


    // Constructors

        //- Construct from fvMesh
        explicit multiphaseSystem(const fvMesh& mesh);


    //- Destructor
    virtual ~multiphaseSystem() = default;


    // Selectors

        static autoPtr<multiphaseSystem> New
        (
            const fvMesh& mesh
        );


    // Member Functions

        using phaseSystem::sigma;
        using phaseSystem::dmdts;

        //- Return the surface tension force
        tmp<surfaceScalarField> surfaceTension(const phaseModel& phase) const;

        //- Indicator of the proximity of the interface
        //  Field values are 1 near and 0 away for the interface.
        tmp<volScalarField> nearInterface() const;

        //- Solve for the phase fractions
        virtual void solve();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
