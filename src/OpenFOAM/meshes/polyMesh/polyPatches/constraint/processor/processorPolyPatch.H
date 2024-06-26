/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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
    Foam::processorPolyPatch

Description
    Neighbour processor patch.

    Note: morph patch face ordering tries to do a geometric ordering.
    (assumes faces coincident) Hence will have problems when cyclics
    are present.

SourceFiles
    processorPolyPatch.C

\*---------------------------------------------------------------------------*/

#ifndef Foam_processorPolyPatch_H
#define Foam_processorPolyPatch_H

#include "meshes/polyMesh/polyPatches/basic/coupled/coupledPolyPatch.H"
#include "meshes/polyMesh/polyBoundaryMesh/polyBoundaryMesh.H"
#include "meshes/meshShapes/face/faceListFwd.H"
#include "meshes/polyMesh/polyMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class processorPolyPatch Declaration
\*---------------------------------------------------------------------------*/

class processorPolyPatch
:
    public coupledPolyPatch
{
    // Private Data

        int myProcNo_;
        int neighbProcNo_;

        //- Processor-neighbour patch face centres
        vectorField neighbFaceCentres_;

        //- Processor-neighbour patch face areas
        vectorField neighbFaceAreas_;

        //- Processor-neighbour patch neighbour cell centres
        vectorField neighbFaceCellCentres_;

        //- Corresponding neighbouring local point label for every local point
        //  (so localPoints()[i] == neighb.localPoints()[neighbPoints_[i]])
        mutable autoPtr<labelList> neighbPointsPtr_;

        //- Corresponding neighbouring local edge label for every local edge
        //  (so edges()[i] == neighb.edges()[neighbEdges_[i]])
        mutable autoPtr<labelList> neighbEdgesPtr_;

protected:

    // Protected constructors

        //- Construct from components with specified name
        processorPolyPatch
        (
            const word& name,
            const label size,
            const label start,
            const label index,
            const polyBoundaryMesh& bm,
            const int myProcNo,
            const int neighbProcNo,
            const transformType transform = UNKNOWN,  // transformation type
            const word& patchType = typeName
        );


    // Protected Member functions

        //- Initialise the calculation of the patch geometry
        void initGeometry(PstreamBuffers&);

        //- Calculate the patch geometry
        void calcGeometry(PstreamBuffers&);

        //- Calculate the patch geometry with externally
        //  provided geometry
        virtual void calcGeometry
        (
            const primitivePatch& referPatch,
            const pointField& thisCtrs,
            const vectorField& thisAreas,
            const pointField& thisCc,
            const pointField& nbrCtrs,
            const vectorField& nbrAreas,
            const pointField& nbrCc
        )
        {
            NotImplemented;
        }

        //- Initialise the patches for moving points
        void initMovePoints(PstreamBuffers&, const pointField&);

        //- Correct patches after moving points
        void movePoints(PstreamBuffers&, const pointField&);

        //- Initialise the update of the patch topology
        virtual void initUpdateMesh(PstreamBuffers&);

        //- Update of the patch topology
        virtual void updateMesh(PstreamBuffers&);


public:

    //- Runtime type information
    TypeName("processor");


    // Constructors

        //- Construct from components with automatically generated standard name
        processorPolyPatch
        (
            const label size,
            const label start,
            const label index,
            const polyBoundaryMesh& bm,
            const int myProcNo,
            const int neighbProcNo,
            const transformType transform = UNKNOWN,  // transformation type
            const word& patchType = typeName
        );

        //- Construct from dictionary
        processorPolyPatch
        (
            const word& name,
            const dictionary& dict,
            const label index,
            const polyBoundaryMesh&,
            const word& patchType
        );

        //- Construct as copy, resetting the boundary mesh
        processorPolyPatch(const processorPolyPatch&, const polyBoundaryMesh&);

        //- Construct as given the original patch and resetting the
        //  face list and boundary mesh information
        processorPolyPatch
        (
            const processorPolyPatch& pp,
            const polyBoundaryMesh& bm,
            const label index,
            const label newSize,
            const label newStart
        );

        //- Construct given the original patch and a map
        processorPolyPatch
        (
            const processorPolyPatch& pp,
            const polyBoundaryMesh& bm,
            const label index,
            const labelUList& mapAddressing,
            const label newStart
        );

        //- Construct and return a clone, resetting the boundary mesh
        virtual autoPtr<polyPatch> clone(const polyBoundaryMesh& bm) const
        {
            return autoPtr<polyPatch>(new processorPolyPatch(*this, bm));
        }

        //- Construct and return a clone, resetting the face list
        //  and boundary mesh
        virtual autoPtr<polyPatch> clone
        (
            const polyBoundaryMesh& bm,
            const label index,
            const label newSize,
            const label newStart
        ) const
        {
            return autoPtr<polyPatch>
            (
                new processorPolyPatch
                (
                    *this,
                    bm,
                    index,
                    newSize,
                    newStart
                )
            );
        }

        //- Construct and return a clone, resetting the face list
        //  and boundary mesh
        virtual autoPtr<polyPatch> clone
        (
            const polyBoundaryMesh& bm,
            const label index,
            const labelUList& mapAddressing,
            const label newStart
        ) const
        {
            return autoPtr<polyPatch>
            (
                new processorPolyPatch
                (
                    *this,
                    bm,
                    index,
                    mapAddressing,
                    newStart
                )
            );
        }


    //- Destructor
    virtual ~processorPolyPatch();


    // Static Functions

        //- Return the name of a processorPolyPatch ("procBoundary..")
        //- constructed from the pair of processor IDs.
        static word newName
        (
            const label myProcNo,
            const label neighbProcNo
        );


    // Member Functions

        //- Return true only if this is a parallel run
        virtual bool coupled() const
        {
            return UPstream::parRun();
        }

        //- Return processor number
        int myProcNo() const noexcept
        {
            return myProcNo_;
        }

        //- Return neighbour processor number
        int neighbProcNo() const noexcept
        {
            return neighbProcNo_;
        }

        //- Does the processor own the patch ?
        virtual bool owner() const
        {
            return (myProcNo_ < neighbProcNo_);
        }

        //- Is the processor the patch neighbour ?
        bool neighbour() const
        {
            return !owner();
        }

        //- Return processor-neighbour patch face centres
        const vectorField& neighbFaceCentres() const noexcept
        {
            return neighbFaceCentres_;
        }

        //- Return processor-neighbour patch face areas
        const vectorField& neighbFaceAreas() const noexcept
        {
            return neighbFaceAreas_;
        }

        //- Return processor-neighbour patch neighbour cell centres
        const vectorField& neighbFaceCellCentres() const noexcept
        {
            return neighbFaceCellCentres_;
        }

        //- Return neighbour point labels. WIP.
        const labelList& neighbPoints() const;

        //- Return neighbour edge labels. WIP.
        const labelList& neighbEdges() const;

        //- Return message tag to use for communication
        virtual int tag() const
        {
            return UPstream::msgType();
        }

        //- Return communicator used for communication
        virtual label comm() const
        {
            return boundaryMesh().mesh().comm();
        }

        //- Transform a patch-based position from other side to this side
        virtual void transformPosition(pointField& l) const
        {}

        //- Transform a patch-based position from other side to this side
        virtual void transformPosition(point&, const label facei) const
        {}

        //- Initialize ordering for primitivePatch. Does not
        //  refer to *this (except for name() and type() etc.)
        virtual void initOrder(PstreamBuffers&, const primitivePatch&) const;

        //- Returns rotation.
        //    -1 : no match
        //     0 : match
        //    >0 : match if rotated clockwise by this amount
        static label matchFace
        (
            const face& localFace,
            const pointField& localPts,
            const face& masterFace,
            const pointField& masterPts,
            const bool sameOrientation,
            const scalar absTolSqr,
            scalar& matchDistSqr
        );

        //- Return new ordering for primitivePatch.
        //  Ordering is -faceMap: for every face
        //  index of the new face -rotation:for every new face the clockwise
        //  shift of the original face. Return false if nothing changes
        //  (faceMap is identity, rotation is 0), true otherwise.
        virtual bool order
        (
            PstreamBuffers&,
            const primitivePatch&,
            labelList& faceMap,
            labelList& rotation
        ) const;


        //- Write the polyPatch data as a dictionary
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
