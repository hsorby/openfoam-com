/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2021 OpenCFD Ltd.
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
    Foam::fvMeshAdder

Description
    Adds two fvMeshes without using any polyMesh morphing.
    Uses polyMeshAdder.

SourceFiles
    fvMeshAdder.C
    fvMeshAdderTemplates.C

\*---------------------------------------------------------------------------*/

#ifndef fvMeshAdder_H
#define fvMeshAdder_H

#include "polyMeshAdder/polyMeshAdder.H"
#include "fields/volFields/volFieldsFwd.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldsFwd.H"
#include "fields/fvsPatchFields/fvsPatchField/fvsPatchFieldsFwd.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldMapper.H"
#include "fields/DimensionedFields/DimensionedField/DimensionedField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward Declarations
class IOobject;
class faceCoupleInfo;
class IOobjectList;
class fvMesh;
class volMesh;
class surfaceMesh;
class mapAddedPolyMesh;

/*---------------------------------------------------------------------------*\
                           Class fvMeshAdder Declaration
\*---------------------------------------------------------------------------*/

class fvMeshAdder
:
    public polyMeshAdder
{
private:

    // Private Member Functions

        //- Calculate map from new patch faces to old patch faces. -1 where
        //  could not map.
        static labelList calcPatchMap
        (
            const label oldStart,
            const label oldSize,
            const labelList& oldToNew,
            const polyPatch& newPatch,
            const label unmappedIndex
        );

        //- Update single volField.
        template<class Type>
        static void MapVolField
        (
            const mapAddedPolyMesh& meshMap,

            GeometricField<Type, fvPatchField, volMesh>& fld,
            const GeometricField<Type, fvPatchField, volMesh>& fldToAdd,
            const bool fullyMapped
        );

        //- Update single surfaceField.
        template<class Type>
        static void MapSurfaceField
        (
            const mapAddedPolyMesh& meshMap,

            GeometricField<Type, fvsPatchField, surfaceMesh>& fld,
            const GeometricField<Type, fvsPatchField, surfaceMesh>& fldToAdd,
            const bool fullyMapped
        );

        //- Update single dimensionedField.
        template<class Type>
        static void MapDimField
        (
            const mapAddedPolyMesh& meshMap,

            DimensionedField<Type, volMesh>& fld,
            const DimensionedField<Type, volMesh>& fldToAdd
        );

public:

    // Declare name of the class and its debug switch
    ClassName("fvMeshAdder");


    // Member Functions

        //- Inplace add mesh to fvMesh. Maps all stored fields. Returns map.
        static autoPtr<mapAddedPolyMesh> add
        (
            fvMesh& mesh0,
            const fvMesh& mesh1,
            const faceCoupleInfo& coupleInfo,
            const bool validBoundary = true,
            const bool fullyMapped = false
        );

        //- Map all volFields of Type.
        //  Optionally override mapping detection
        //  of unmapped values (e.g. used in fvMeshDistribute since it fixes
        //  up mapping itself)
        template<class Type>
        static void MapVolFields
        (
            const mapAddedPolyMesh&,
            const fvMesh& mesh,
            const fvMesh& meshToAdd,
            const bool fullyMapped = false
        );

        //- Map all surfaceFields of Type
        template<class Type>
        static void MapSurfaceFields
        (
            const mapAddedPolyMesh&,
            const fvMesh& mesh,
            const fvMesh& meshToAdd,
            const bool fullyMapped = false
        );

        //- Map all DimensionedFields of Type
        template<class Type>
        static void MapDimFields
        (
            const mapAddedPolyMesh&,
            const fvMesh& mesh,
            const fvMesh& meshToAdd
        );


        // Multi-mesh merging. Expects same patches (apart from processor
        // patches). Adds to element 0 of flds, meshes

            //- In-place add to fvMeshes[myProci]. Stitch boundary faces.
            static autoPtr<mapPolyMesh> add
            (
                const label myProci,    // index of mesh to modify (== mesh_)
                UPtrList<fvMesh>& fvMeshes,
                const labelList& oldFaceOwner,  // face owner for myProci mesh

                // Coupling info
                const labelListList& localBoundaryFace,
                const labelListList& remoteFaceProc,
                const labelListList& remoteBoundaryFace,

                labelListList& constructPatchMap,
                labelListList& constructCellMap,
                labelListList& constructFaceMap,
                labelListList& constructPointMap
            );

            //- Update single dimensionedField.
            template<class Type>
            static void MapDimField
            (
                UPtrList<DimensionedField<Type, volMesh>>& flds,
                const labelListList& cellProcAddressing,
                const bool fullyMapped
            );

            //- Update single volField.
            template<class Type>
            static void MapVolField
            (
                UPtrList<GeometricField<Type, fvPatchField, volMesh>>& flds,
                const labelList& oldPatchStarts0,
                const labelList& oldPatchSizes0,
                const labelListList& patchProcAddressing,
                const labelListList& cellProcAddressing,
                const labelListList& faceProcAddressing,
                const bool fullyMapped
            );

            //- Update single surfaceField.
            template<class Type>
            static void MapSurfaceField
            (
                UPtrList<GeometricField<Type, fvsPatchField, surfaceMesh>>&,
                const labelList& oldFaceOwner0,
                const labelList& oldPatchStarts0,
                const labelList& oldPatchSizes0,
                const labelListList& patchProcAddressing,
                const labelListList& cellProcAddressing,
                const labelListList& faceProcAddressing,
                const bool fullyMapped
            );


            //- Map all dimensionedField of Type.
            //  Optionally override mapping detection
            //  of unmapped values (e.g. used in fvMeshDistribute since it
            //  fixes up mapping itself)
            template<class Type>
            static void MapDimFields
            (
                const UPtrList<fvMesh>& meshes,
                const labelListList& cellProcAddressing,
                const bool fullyMapped = false
            );

            //- Map all volFields of Type.
            //  Optionally override mapping detection
            //  of unmapped values (e.g. used in fvMeshDistribute since it
            //  fixes up mapping itself)
            template<class Type>
            static void MapVolFields
            (
                const UPtrList<fvMesh>& meshes,
                const labelList& oldPatchStarts0,
                const labelList& oldPatchSizes0,
                const labelListList& patchProcAddressing,
                const labelListList& cellProcAddressing,
                const labelListList& faceProcAddressing,
                const labelListList& pointProcAddressing,
                const bool fullyMapped = false
            );

            //- Map all surfaceFields of Type
            template<class Type>
            static void MapSurfaceFields
            (
                const UPtrList<fvMesh>& meshes,
                const labelList& oldFaceOwner0,
                const labelList& oldPatchStarts0,
                const labelList& oldPatchSizes0,
                const labelListList& patchProcAddressing,
                const labelListList& cellProcAddressing,
                const labelListList& faceProcAddressing,
                const labelListList& pointProcAddressing,
                const bool fullyMapped = false
            );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "fvMeshAdder/fvMeshAdderTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
