/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2018 OpenCFD Ltd.
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
    Foam::searchableSurface

Description
    Base class of (analytical or triangulated) surface.
    Encapsulates all the search routines. WIP.

    Information returned is usually a pointIndexHit:
    - bool  : was intersection/nearest found?
    - point : intersection point or nearest point
    - index : unique index on surface (e.g. triangle for triSurfaceMesh)

SourceFiles
    searchableSurface.C

\*---------------------------------------------------------------------------*/

#ifndef Foam_searchableSurface_H
#define Foam_searchableSurface_H

#include "meshes/primitiveShapes/point/pointField.H"
#include "meshes/boundBox/boundBox.H"
#include "db/typeInfo/typeInfo.H"
#include "db/runTimeSelection/construction/runTimeSelectionTables.H"
#include "meshes/primitiveShapes/objectHit/pointIndexHit.H"
#include "meshes/primitiveShapes/line/line.H"
#include "db/objectRegistry/objectRegistry.H"
#include "meshes/primitiveShapes/volumeType/volumeType.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward Declarations
class objectRegistry;
class mapDistribute;
class treeBoundBox;

/*---------------------------------------------------------------------------*\
                      Class searchableSurface Declaration
\*---------------------------------------------------------------------------*/

class searchableSurface
:
    public regIOobject
{
    // Private data

        const word name_;

        boundBox bounds_;


    // Private Member Functions

        //- No copy construct
        searchableSurface(const searchableSurface&) = delete;

        //- No copy assignment
        void operator=(const searchableSurface&) = delete;


public:

    //- Runtime type information
    TypeName("searchableSurface");

    // Declare run-time constructor selection table

        // For the dictionary constructor
        declareRunTimeSelectionTable
        (
            autoPtr,
            searchableSurface,
            dict,
            (
                const IOobject& io,
                const dictionary& dict
            ),
            (io, dict)
        );


        //- Class used for the read-construction of
        //  PtrLists of searchableSurface.
        class iNew
        {
            IOobject& io_;

        public:

            iNew(IOobject& io)
            :
                io_(io)
            {}

            autoPtr<searchableSurface> operator()(Istream& is) const
            {
                word surfaceType(is);
                word readName(is);
                dictionary dict(is);

                autoPtr<IOobject> namedIO(io_.clone());
                namedIO().rename(readName);
                return searchableSurface::New(surfaceType, namedIO(), dict);
            }
        };


    // Constructors

        searchableSurface(const IOobject& io);

        //- Clone
        virtual autoPtr<searchableSurface> clone() const
        {
            NotImplemented;
            return nullptr;
        }


    // Selectors

        //- Return a reference to the selected searchableSurface
        static autoPtr<searchableSurface> New
        (
            const word& surfaceType,
            const IOobject& io,
            const dictionary& dict
        );


    //- Destructor
    virtual ~searchableSurface() = default;


    // Member Functions

        //- Is object global
        virtual bool global() const
        {
            return true;
        }

        //- Return complete path + object name if the file exists
        //  either in the case/processor or case otherwise null
        virtual fileName filePath() const
        {
            return globalFilePath(type());
        }

        //- Return const reference to boundBox
        virtual const boundBox& bounds() const
        {
            return bounds_;
        }

        //- Return non-const access to the boundBox to allow it to be set.
        virtual boundBox& bounds()
        {
            return bounds_;
        }

        //- Names of regions
        virtual const wordList& regions() const = 0;

        //- Whether supports volume type (below).
        //  This is false for the base class.
        virtual bool hasVolumeType() const;

        //- If surface supports volume queries, what is type of points outside
        //  bounds
        virtual volumeType outsideVolumeType() const = 0;

        //- Range of local indices that can be returned
        virtual label size() const = 0;

        //- Range of global indices that can be returned
        virtual label globalSize() const
        {
            return size();
        }

        //- Get representative set of element coordinates
        //  Usually the element centres (should be of length size()).
        virtual tmp<pointField> coordinates() const = 0;

        //- Get bounding spheres (centre and radius squared), one per element.
        //  Any point on element is guaranteed to be inside.
        virtual void boundingSpheres
        (
            pointField& centres,
            scalarField& radiusSqr
        ) const = 0;

        //- Get the points that define the surface.
        virtual tmp<pointField> points() const = 0;

        //- Does any part of the surface overlap the supplied bound box?
        virtual bool overlaps(const boundBox& bb) const = 0;

        // Single point queries.

            ////- Calculate nearest point on surface. Returns
            ////  - bool : any point found nearer than nearestDistSqr
            ////  - label: relevant index in surface
            ////  - label: region in surface
            ////  - point: actual nearest point found
            //virtual pointIndexHit findNearest
            //(
            //    const point& sample,
            //    const scalar nearestDistSqr
            //) const = 0;
            //
            ////- Calculate nearest point on edge. Returns
            ////  - bool : any point found nearer than nearestDistSqr
            ////  - label: relevant index in surface
            ////  - label: region in surface
            ////  - point: actual nearest point found
            //virtual pointIndexHit findNearestOnEdge
            //(
            //    const point& sample,
            //    const scalar nearestDistSqr
            //) const = 0;
            //
            ////- Find nearest to segment. Returns
            ////  - bool : any point found?
            ////  - label: relevant index in shapes
            ////  - label: region in surface
            ////  - point: actual nearest point found
            ////  sets:
            ////  - tightest  : bounding box
            ////  - linePoint : corresponding nearest point on line
            //virtual pointIndexHit findNearest
            //(
            //    const linePointRef& ln,
            //    treeBoundBox& tightest,
            //    point& linePoint
            //) const = 0;
            //
            ////- Find nearest intersection of line between start and end.
            //virtual pointIndexHit findLine
            //(
            //    const point& start,
            //    const point& end
            //) const = 0;
            //
            ////- Find any intersection of line between start and end.
            //virtual pointIndexHit findLineAny
            //(
            //    const point& start,
            //    const point& end
            //) const = 0;


        // Multiple point queries. When surface is distributed the index
        // should be a global index. Not done yet.

            virtual void findNearest
            (
                const pointField& sample,
                const scalarField& nearestDistSqr,
                List<pointIndexHit>&
            ) const = 0;

            //- Find the nearest locations for the supplied points to a
            //  particular region in the searchable surface.
            virtual void findNearest
            (
                const pointField& samples,
                const scalarField& nearestDistSqr,
                const labelList& regionIndices,
                List<pointIndexHit>& info
            ) const
            {
                findNearest(samples, nearestDistSqr, info);
            }

            //- Find first intersection on segment from start to end.
            //  Note: searchableSurfacesQueries expects no
            //  intersection to be found if start==end. Is problem?
            virtual void findLine
            (
                const pointField& start,
                const pointField& end,
                List<pointIndexHit>&
            ) const = 0;

            //- Return any intersection on segment from start to end.
            virtual void findLineAny
            (
                const pointField& start,
                const pointField& end,
                List<pointIndexHit>&
            ) const = 0;

            //- Get all intersections in order from start to end.
            virtual void findLineAll
            (
                const pointField& start,
                const pointField& end,
                List<List<pointIndexHit>>&
            ) const = 0;

            //- From a set of points and indices get the region
            virtual void getRegion
            (
                const List<pointIndexHit>&,
                labelList& region
            ) const = 0;

            //- From a set of points and indices get the normal
            virtual void getNormal
            (
                const List<pointIndexHit>&,
                vectorField& normal
            ) const = 0;

            //- Determine type (inside/outside) for point.
            //  Unknown if cannot be determined (e.g. non-manifold surface)
            virtual void getVolumeType
            (
                const pointField&,
                List<volumeType>&
            ) const = 0;

            //- Find nearest, normal and region. Can be overridden with
            //  optimised implementation
            virtual void findNearest
            (
                const pointField& sample,
                const scalarField& nearestDistSqr,
                List<pointIndexHit>&,
                vectorField& normal,
                labelList& region
            ) const;


        // Other

            //- Set bounds of surface. Bounds currently set as list of
            //  bounding boxes. The bounds are hints to the surface as for
            //  the range of queries it can expect. faceMap/pointMap can be
            //  set if the surface has done any redistribution.
            virtual void distribute
            (
                const List<treeBoundBox>&,
                const bool keepNonLocal,
                autoPtr<mapDistribute>& faceMap,
                autoPtr<mapDistribute>& pointMap
            )
            {}

            //- WIP. Store element-wise field.
            virtual void setField(const labelList& values)
            {}

            //- WIP. From a set of hits (points and
            //  indices) get the specified field. Misses do not get set. Return
            //  empty field if not supported.
            virtual void getField(const List<pointIndexHit>&, labelList& values)
            const
            {
                values.clear();
            }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Global file type for searchableSurface
template<>
struct is_globalIOobject<searchableSurface> : std::true_type {};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
