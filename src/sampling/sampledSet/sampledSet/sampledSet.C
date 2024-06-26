/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2018-2022 OpenCFD Ltd.
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

#include "sampledSet/sampledSet/sampledSet.H"
#include "meshes/polyMesh/polyMesh.H"
#include "meshes/primitiveMesh/primitiveMesh.H"
#include "meshSearch/meshSearch.H"
#include "particle/particle.H"
#include "parallel/globalIndex/globalIndex.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(sampledSet, 0);
    defineRunTimeSelectionTable(sampledSet, word);
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void Foam::sampledSet::checkDimensions() const
{
    if
    (
        (cells_.size() != size())
     || (faces_.size() != size())
     || (segments_.size() != size())
     || (distance_.size() != size())
    )
    {
        FatalErrorInFunction
            << "Sizes not equal : "
            << "  points:" << size()
            << "  cells:" << cells_.size()
            << "  faces:" << faces_.size()
            << "  segments:" << segments_.size()
            << "  distance:" << distance_.size()
            << abort(FatalError);
    }
}


Foam::label Foam::sampledSet::getBoundaryCell(const label facei) const
{
    return mesh().faceOwner()[facei];
}


Foam::label Foam::sampledSet::getNeighbourCell(const label facei) const
{
    if (facei < mesh().nInternalFaces())
    {
        return mesh().faceNeighbour()[facei];
    }

    return mesh().faceOwner()[facei];
}


Foam::label Foam::sampledSet::pointInCell
(
    const point& p,
    const label samplei
) const
{
    // Collect the face owner and neighbour cells of the sample into an array
    // for convenience
    const label cells[4] =
    {
        mesh().faceOwner()[faces_[samplei]],
        getNeighbourCell(faces_[samplei]),
        mesh().faceOwner()[faces_[samplei+1]],
        getNeighbourCell(faces_[samplei+1])
    };

    // Find the sampled cell by checking the owners and neighbours of the
    // sampled faces
    label cellm =
        (cells[0] == cells[2] || cells[0] == cells[3]) ? cells[0]
      : (cells[1] == cells[2] || cells[1] == cells[3]) ? cells[1]
      : -1;

    if (cellm != -1)
    {
        // If found the sampled cell check the point is in the cell
        // otherwise ignore
        if (!mesh().pointInCell(p, cellm, searchEngine_.decompMode()))
        {
            cellm = -1;

            if (debug)
            {
                WarningInFunction
                    << "Could not find mid-point " << p
                    << " cell " << cellm << endl;
            }
        }
    }
    else
    {
        // If the sample does not pass through a single cell check if the point
        // is in any of the owners or neighbours otherwise ignore
        for (label i=0; i<4; ++i)
        {
            if (mesh().pointInCell(p, cells[i], searchEngine_.decompMode()))
            {
                return cells[i];
            }
        }

        if (debug)
        {
            WarningInFunction
                << "Could not find cell for mid-point" << nl
                << "  samplei: " << samplei
                << "  pts[samplei]: " << operator[](samplei)
                << "  face[samplei]: " << faces_[samplei]
                << "  pts[samplei+1]: " << operator[](samplei+1)
                << "  face[samplei+1]: " << faces_[samplei+1]
                << "  cellio: " << cells[0]
                << "  cellin: " << cells[1]
                << "  celljo: " << cells[2]
                << "  celljn: " << cells[3]
                << endl;
        }
    }

    return cellm;
}


Foam::scalar Foam::sampledSet::calcSign
(
    const label facei,
    const point& sample
) const
{
    vector vec = sample - mesh().faceCentres()[facei];

    scalar magVec = mag(vec);

    if (magVec < VSMALL)
    {
        // Sample on face centre. Regard as inside
        return -1;
    }

    vec /= magVec;

    const vector n = normalised(mesh().faceAreas()[facei]);

    return n & vec;
}


Foam::label Foam::sampledSet::findNearFace
(
    const label celli,
    const point& sample,
    const scalar smallDist
) const
{
    const cell& myFaces = mesh().cells()[celli];

    forAll(myFaces, myFacei)
    {
        const face& f = mesh().faces()[myFaces[myFacei]];

        pointHit inter = f.nearestPoint(sample, mesh().points());

        scalar dist = inter.point().dist(sample);

        if (dist < smallDist)
        {
            return myFaces[myFacei];
        }
    }
    return -1;
}


Foam::point Foam::sampledSet::pushIn
(
    const point& facePt,
    const label facei
) const
{
    label celli = mesh().faceOwner()[facei];
    const point& cC = mesh().cellCentres()[celli];

    point newPosition = facePt;

    // Taken from particle::initCellFacePt()
    label tetFacei;
    label tetPtI;
    mesh().findTetFacePt(celli, facePt, tetFacei, tetPtI);

    // This is the tolerance that was defined as a static constant of the
    // particle class. It is no longer used by particle, following the switch to
    // barycentric tracking. The only place in which the tolerance is now used
    // is here. I'm not sure what the purpose of this code is, but it probably
    // wants removing. It is doing tet-searches for a particle position. This
    // should almost certainly be left to the particle class.
    const scalar trackingCorrectionTol = 1e-5;

    if (tetFacei == -1 || tetPtI == -1)
    {
        newPosition = facePt;

        label trap(1.0/trackingCorrectionTol + 1);

        label iterNo = 0;

        do
        {
            newPosition += trackingCorrectionTol*(cC - facePt);

            mesh().findTetFacePt
            (
                celli,
                newPosition,
                tetFacei,
                tetPtI
            );

            ++iterNo;

        } while (tetFacei < 0  && iterNo <= trap);
    }

    if (tetFacei == -1)
    {
        FatalErrorInFunction
            << "After pushing " << facePt << " to " << newPosition
            << " it is still outside face " << facei
            << " at " << mesh().faceCentres()[facei]
            << " of cell " << celli
            << " at " << cC << endl
            << "Please change your starting point"
            << abort(FatalError);
    }

    return newPosition;
}


bool Foam::sampledSet::getTrackingPoint
(
    const point& samplePt,
    const point& bPoint,
    const label bFacei,
    const scalar smallDist,

    point& trackPt,
    label& trackCelli,
    label& trackFacei
) const
{
    bool isGoodSample = false;

    if (bFacei == -1)
    {
        // No boundary intersection. Try and find cell samplePt is in
        trackCelli = mesh().findCell(samplePt, searchEngine_.decompMode());

        if
        (
            (trackCelli == -1)
        || !mesh().pointInCell
            (
                samplePt,
                trackCelli,
                searchEngine_.decompMode()
            )
        )
        {
            // Line samplePt - end_ does not intersect domain at all.
            // (or is along edge)

            trackCelli = -1;
            trackFacei = -1;

            isGoodSample = false;
        }
        else
        {
            // Start is inside. Use it as tracking point

            trackPt = samplePt;
            trackFacei = -1;

            isGoodSample = true;
        }
    }
    else if (mag(samplePt - bPoint) < smallDist)
    {
        // samplePt close to bPoint. Snap to it
        trackPt = pushIn(bPoint, bFacei);
        trackFacei = bFacei;
        trackCelli = getBoundaryCell(trackFacei);

        isGoodSample = true;
    }
    else
    {
        scalar sign = calcSign(bFacei, samplePt);

        if (sign < 0)
        {
            // samplePt inside or marginally outside.
            trackPt = samplePt;
            trackFacei = -1;
            trackCelli = mesh().findCell(trackPt, searchEngine_.decompMode());

            isGoodSample = true;
        }
        else
        {
            // samplePt outside. use bPoint
            trackPt = pushIn(bPoint, bFacei);
            trackFacei = bFacei;
            trackCelli = getBoundaryCell(trackFacei);

            isGoodSample = false;
        }
    }

    DebugInFunction
        << " samplePt:" << samplePt
        << " bPoint:" << bPoint
        << " bFacei:" << bFacei << nl
        << "   Calculated first tracking point :"
        << " trackPt:" << trackPt
        << " trackCelli:" << trackCelli
        << " trackFacei:" << trackFacei
        << " isGoodSample:" << isGoodSample
        << endl;

    return isGoodSample;
}


void Foam::sampledSet::setSamples
(
    const List<point>& samplingPts,
    const labelList& samplingCells,
    const labelList& samplingFaces,
    const labelList& samplingSegments,
    const scalarList& samplingDistance
)
{
    setPoints(samplingPts);
    setDistance(samplingDistance, false);  // check=false

    segments_ = samplingSegments;
    cells_ = samplingCells;
    faces_ = samplingFaces;

    checkDimensions();
}


void Foam::sampledSet::setSamples
(
    List<point>&& samplingPts,
    labelList&& samplingCells,
    labelList&& samplingFaces,
    labelList&& samplingSegments,
    scalarList&& samplingDistance
)
{
    setPoints(std::move(samplingPts));
    setDistance(std::move(samplingDistance), false);  // check=false

    segments_ = std::move(samplingSegments);
    cells_ = std::move(samplingCells);
    faces_ = std::move(samplingFaces);

    checkDimensions();
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::sampledSet::sampledSet
(
    const word& name,
    const polyMesh& mesh,
    const meshSearch& searchEngine,
    const coordSet::coordFormat axisType
)
:
    coordSet(name, axisType),
    mesh_(mesh),
    searchEngine_(searchEngine),
    segments_(),
    cells_(),
    faces_()
{}


Foam::sampledSet::sampledSet
(
    const word& name,
    const polyMesh& mesh,
    const meshSearch& searchEngine,
    const word& axis
)
:
    coordSet(name, axis),
    mesh_(mesh),
    searchEngine_(searchEngine),
    segments_(),
    cells_(),
    faces_()
{}


Foam::sampledSet::sampledSet
(
    const word& name,
    const polyMesh& mesh,
    const meshSearch& searchEngine,
    const dictionary& dict
)
:
    coordSet(name, dict.get<word>("axis")),
    mesh_(mesh),
    searchEngine_(searchEngine),
    segments_(),
    cells_(),
    faces_()
{}


// Foam::autoPtr<Foam::coordSet> Foam::sampledSet::gather
// (
//     labelList& sortOrder,
//     labelList& allSegments
// ) const
// {
//     autoPtr<coordSet> result(coordSet::gatherSort(sortOrder));
//
//     // Optional
//     if (notNull(allSegments))
//     {
//         globalIndex::gatherOp(segments(), allSegments);
//         allSegments = UIndirectList<label>(allSegments, sortOrder)();
//     }
//
//     return result;
// }


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::autoPtr<Foam::sampledSet> Foam::sampledSet::New
(
    const word& name,
    const polyMesh& mesh,
    const meshSearch& searchEngine,
    const dictionary& dict
)
{
    const word sampleType(dict.get<word>("type"));

    auto* ctorPtr = wordConstructorTable(sampleType);

    if (!ctorPtr)
    {
        FatalIOErrorInLookup
        (
            dict,
            "sample",
            sampleType,
            *wordConstructorTablePtr_
        ) << exit(FatalIOError);
    }

    return autoPtr<sampledSet>
    (
        ctorPtr
        (
            name,
            mesh,
            searchEngine,
            dict.optionalSubDict(sampleType + "Coeffs")
        )
    );
}


Foam::Ostream& Foam::sampledSet::write(Ostream& os) const
{
    coordSet::write(os);

    os  << nl << "\t(celli)\t(facei)" << nl;

    forAll(*this, samplei)
    {
        os  << '\t' << cells_[samplei]
            << '\t' << faces_[samplei]
            << nl;
    }

    return os;
}


// ************************************************************************* //
