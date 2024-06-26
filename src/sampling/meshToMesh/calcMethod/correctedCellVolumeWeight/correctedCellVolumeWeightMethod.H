/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2015 OpenFOAM Foundation
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
    Foam::correctedCellVolumeWeightMethod

Description
    Cell-volume-weighted mesh-to-mesh interpolation class

    Volume conservative with calculated offset vectors

SourceFiles
    correctedCellVolumeWeightMethod.C

\*---------------------------------------------------------------------------*/

#ifndef correctedCellVolumeWeightMethod_H
#define correctedCellVolumeWeightMethod_H

#include "meshToMesh/calcMethod/cellVolumeWeight/cellVolumeWeightMethod.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                   Class correctedCellVolumeWeightMethod Declaration
\*---------------------------------------------------------------------------*/

class correctedCellVolumeWeightMethod
:
    public cellVolumeWeightMethod
{
protected:

    // Protected Member Functions

        //- Calculate the mesh-to-mesh addressing and weights
        void calculateAddressing
        (
            labelListList& srcToTgtCellAddr,
            scalarListList& srcToTgtCellWght,
            pointListList& srcToTgtCellVec,
            labelListList& tgtToSrcCellAddr,
            scalarListList& tgtToSrcCellWght,
            pointListList& tgtToSrcCellVec,
            const label srcSeedI,
            const label tgtSeedI,
            const labelList& srcCellIDs,
            boolList& mapFlag,
            label& startSeedI
        );

        //- No copy construct
        correctedCellVolumeWeightMethod
        (
            const correctedCellVolumeWeightMethod&
        ) = delete;

        //- No copy assignment
        void operator=(const correctedCellVolumeWeightMethod&) = delete;


public:

    //- Run-time type information
    TypeName("correctedCellVolumeWeight");

    //- Construct from source and target meshes
    correctedCellVolumeWeightMethod(const polyMesh& src, const polyMesh& tgt);

    //- Destructor
    virtual ~correctedCellVolumeWeightMethod();


    // Member Functions

        // Evaluate

            //- Calculate addressing and weights and optionally offset vectors
            virtual void calculate
            (
                labelListList& srcToTgtAddr,
                scalarListList& srcToTgtWght,
                pointListList& srcToTgtVec,
                labelListList& tgtToSrcAddr,
                scalarListList& tgtToSrcWght,
                pointListList& tgtToSrcVec
            );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
