/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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

Class
    Foam::moleculeCloud

Description

SourceFiles
    moleculeCloudI.H
    moleculeCloud.C

\*---------------------------------------------------------------------------*/

#ifndef Foam_moleculeCloud_H
#define Foam_moleculeCloud_H

#include "Cloud/CloudPascal.H"
#include "molecule/molecule.H"
#include "db/IOobjects/IOdictionary/IOdictionary.H"
#include "potential/potential.H"
#include "InteractionLists/InteractionLists.H"
#include "primitives/Vector/ints/labelVector.H"
#include "primitives/random/Random/Random.H"
#include "primitives/strings/fileName/fileName.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class moleculeCloud Declaration
\*---------------------------------------------------------------------------*/

class moleculeCloud
:
    public Cloud<molecule>
{
    // Private Data

        const polyMesh& mesh_;

        const potential& pot_;

        List<DynamicList<molecule*>> cellOccupancy_;

        InteractionLists<molecule> il_;

        List<molecule::constantProperties> constPropList_;

        Random rndGen_;


    // Private Member Functions

        void buildConstProps();

        void setSiteSizesAndPositions();

        //- Determine which molecules are in which cells
        void buildCellOccupancy();

        void calculatePairForce();

        inline void evaluatePair
        (
            molecule& molI,
            molecule& molJ
        );

        inline bool evaluatePotentialLimit
        (
            molecule& molI,
            molecule& molJ
        ) const;

        void calculateTetherForce();

        void calculateExternalForce();

        void removeHighEnergyOverlaps();

        void initialiseMolecules
        (
            const IOdictionary& mdInitialiseDict
        );

        void createMolecule
        (
            const point& position,
            label cell,
            label id,
            bool tethered,
            scalar temperature,
            const vector& bulkVelocity
        );

        label nSites() const;

        inline vector equipartitionLinearVelocity
        (
            scalar temperature,
            scalar mass
        );

        inline vector equipartitionAngularMomentum
        (
            scalar temperature,
            const molecule::constantProperties& cP
        );


public:

    //- Type of parcel within the cloud
    typedef molecule parcelType;

    //- No copy construct
    moleculeCloud(const moleculeCloud&) = delete;

    //- No copy assignment
    void operator=(const moleculeCloud&) = delete;


    // Constructors

        //- Read construct given mesh and potential references
        moleculeCloud
        (
            const polyMesh& mesh,
            const potential& pot,
            bool readFields = true
        );

        //- Read construct given mesh, potential and mdInitialiseDict
        moleculeCloud
        (
            const polyMesh& mesh,
            const potential& pot,
            const IOdictionary& mdInitialiseDict,
            bool readFields = true
        );


    // Member Functions

        //- Evolve the molecules (move, calculate forces, control state etc)
        void evolve();

        void calculateForce();

        void applyConstraintsAndThermostats
        (
            const scalar targetTemperature,
            const scalar measuredTemperature
        );


    // Access

        // Same as pMesh()
        const polyMesh& mesh() const noexcept { return mesh_; }

        const potential& pot() const { return pot_; }

        const List<DynamicList<molecule*>>& cellOccupancy() const
        {
            return cellOccupancy_;
        }

        const InteractionLists<molecule>& il() const
        {
            return il_;
        }

        const List<molecule::constantProperties>& constProps() const
        {
            return constPropList_;
        }

        const molecule::constantProperties& constProps(label id) const
        {
            return constPropList_[id];
        }

        Random& rndGen() { return rndGen_; }


    // Member Operators

        //- Write molecule sites in XYZ format
        void writeXYZ(const fileName& fName) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "moleculeCloud/moleculeCloudI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
