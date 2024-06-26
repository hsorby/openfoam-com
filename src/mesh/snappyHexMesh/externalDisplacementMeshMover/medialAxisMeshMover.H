/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2014-2015 OpenFOAM Foundation
    Copyright (C) 2015 OpenCFD Ltd.
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
    Foam::medialAxisMeshMover

Description
    Mesh motion solver that uses a medial axis algorithm to work
    out a fraction between the (nearest point on a) moving surface
    and the (nearest point on a) fixed surface.

    This fraction is then used to scale the motion. Use
    - fixedValue on all moving patches
    - zeroFixedValue on stationary patches
    - slip on all slipping patches
    Note that the fixedValue boundary conditions might be changed by this
    solver to enforce consistency and a valid resulting mesh.

SourceFiles
    medialAxisMeshMover.C

\*---------------------------------------------------------------------------*/

#ifndef medialAxisMeshMover_H
#define medialAxisMeshMover_H

#include "externalDisplacementMeshMover/externalDisplacementMeshMover.H"
#include "motionSmoother/motionSmootherAlgo.H"
#include "snappyHexMeshDriver/snappyLayerDriver.H"
#include "externalDisplacementMeshMover/fieldSmoother/fieldSmoother.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template <class DataType>
class PointData;

/*---------------------------------------------------------------------------*\
             Class medialAxisMeshMover Declaration
\*---------------------------------------------------------------------------*/

class medialAxisMeshMover
:
    public externalDisplacementMeshMover
{
    // Private data

        const labelList adaptPatchIDs_;

        autoPtr<indirectPrimitivePatch> adaptPatchPtr_;

        //- Scale factor for displacement
        pointScalarField scale_;

        //- Starting mesh position
        pointField oldPoints_;

        //- Mesh mover algorithm
        motionSmootherAlgo meshMover_;

        //- Field smoothing
        fieldSmoother fieldSmoother_;


    // Pre-calculated medial axis information

        //- Normal of nearest wall. Where this normal changes direction
        //  defines the medial axis
        pointVectorField dispVec_;

        //- Ratio of medial distance to wall distance.
        //  (1 at wall, 0 at medial axis)
        pointScalarField medialRatio_;

        //- Distance to nearest medial axis point
        pointScalarField medialDist_;

        //- Location on nearest medial axis point
        pointVectorField medialVec_;


    // Private Member Functions

        //- Extract bc types. Replace fixedValue derivatives with fixedValue
        wordList getPatchFieldTypes(const pointVectorField& fld);


        // Calculation of medial axis information

            //- Is mesh edge on a cusp of displacement
            bool isMaxEdge
            (
                const List<PointData<vector>>& pointWallDist,
                const label edgeI,
                const scalar minCos,
                const bool checkWall
            ) const;

            //- Initialise medial axis. Uses pointDisplacement field only
            //  for boundary types, not values.
            void update(const dictionary&);


        // Calculation of mesh movement

            //- Unmark extrusion at given point
            static bool unmarkExtrusion
            (
                const label patchPointi,
                pointField& patchDisp,
                List<snappyLayerDriver::extrudeMode>& extrudeStatus
            );

            //- Synchronise extrusion
            void syncPatchDisplacement
            (
                const scalarField& minThickness,
                pointField& patchDisp,
                List<snappyLayerDriver::extrudeMode>& extrudeStatus
            ) const;

            //- Stop layer growth at feature edges
            void handleFeatureAngleLayerTerminations
            (
                const scalar minCos,
                const bitSet& isMasterPoint,
                const labelList& meshEdges,
                List<snappyLayerDriver::extrudeMode>& extrudeStatus,
                pointField& patchDisp,
                label& nPointCounter
            ) const;

            //- Find isolated islands (points, edges and faces and layer
            //  terminations) in the layer mesh and stop any layer growth
            //  at these points
            void findIsolatedRegions
            (
                const scalar minCosLayerTermination,
                const bool detectExtrusionIsland,
                const bitSet& isMasterPoint,
                const bitSet& isMasterEdge,
                const labelList& meshEdges,
                const scalarField& minThickness,
                List<snappyLayerDriver::extrudeMode>& extrudeStatus,
                pointField& patchDisp
            ) const;


        //- Calculate desired displacement. Modifies per-patch displacement
        //  and calculates displacement for whole mesh
        //  (in pointDisplacement)
        void calculateDisplacement
        (
            const dictionary&,
            const scalarField& minThickness,
            List<snappyLayerDriver::extrudeMode>& extrudeStatus,
            pointField& patchDisp
        );

        //- Move mesh according to calculated displacement
        bool shrinkMesh
        (
            const dictionary& meshQualityDict,
            const label nAllowableErrors,
            labelList& checkFaces
        );

        //- No copy construct
        medialAxisMeshMover(const medialAxisMeshMover&) = delete;

        //- No copy assignment
        void operator=(const medialAxisMeshMover&) = delete;


public:

    //- Runtime type information
    TypeName("displacementMedialAxis");


    // Constructors

        //- Construct from dictionary and displacement field
        medialAxisMeshMover
        (
            const dictionary& dict,
            const List<labelPair>& baffles,
            pointVectorField& pointDisplacement,
            const bool dryRun
        );


    // Destructor

        virtual ~medialAxisMeshMover();


    // Member Functions

        //- Move mesh using current pointDisplacement boundary values.
        //  Return true if successful (errors on checkFaces less than
        //  allowable). Updates pointDisplacement.
        virtual bool move
        (
            const dictionary&,
            const label nAllowableErrors,
            labelList& checkFaces
        );

        //- Update local data for geometry changes
        virtual void movePoints(const pointField&);

        //-  Update local data for topology changes
        virtual void updateMesh(const mapPolyMesh&)
        {
            NotImplemented;
        }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
