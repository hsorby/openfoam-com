/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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

#include "cellSplitter.H"
#include "meshes/polyMesh/polyMesh.H"
#include "polyTopoChange/polyTopoChange.H"
#include "polyTopoChange/addObject/polyAddCell.H"
#include "polyTopoChange/addObject/polyAddFace.H"
#include "polyTopoChange/addObject/polyAddPoint.H"
#include "polyTopoChange/modifyObject/polyModifyFace.H"
#include "meshes/polyMesh/mapPolyMesh/mapPolyMesh.H"
#include "meshTools/meshTools.H"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
defineTypeNameAndDebug(cellSplitter, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::cellSplitter::getFaceInfo
(
    const label facei,
    label& patchID,
    label& zoneID,
    label& zoneFlip
) const
{
    patchID = -1;

    if (!mesh_.isInternalFace(facei))
    {
        patchID = mesh_.boundaryMesh().whichPatch(facei);
    }

    zoneID = mesh_.faceZones().whichZone(facei);

    zoneFlip = false;

    if (zoneID >= 0)
    {
        const faceZone& fZone = mesh_.faceZones()[zoneID];

        zoneFlip = fZone.flipMap()[fZone.whichFace(facei)];
    }
}


// Find the new owner of facei (since the original cell has been split into
// newCells
Foam::label Foam::cellSplitter::newOwner
(
    const label facei,
    const Map<labelList>& cellToCells
) const
{
    const label old = mesh_.faceOwner()[facei];

    const auto iter = cellToCells.cfind(old);

    if (!iter.good())
    {
        // Unsplit cell
        return old;
    }


    // Look up index of face in the cells' faces.

    const labelList& newCells = *iter;

    const cell& cFaces = mesh_.cells()[old];

    return newCells[cFaces.find(facei)];
}


Foam::label Foam::cellSplitter::newNeighbour
(
    const label facei,
    const Map<labelList>& cellToCells
) const
{
    const label old = mesh_.faceNeighbour()[facei];

    const auto iter = cellToCells.cfind(old);

    if (!iter.good())
    {
        // Unsplit cell
        return old;
    }


    // Look up index of face in the cells' faces.

    const labelList& newCells = *iter;

    const cell& cFaces = mesh_.cells()[old];

    return newCells[cFaces.find(facei)];
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::cellSplitter::cellSplitter(const polyMesh& mesh)
:
    mesh_(mesh),
    addedPoints_()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::cellSplitter::setRefinement
(
    const Map<point>& cellToMidPoint,
    polyTopoChange& meshMod
)
{
    addedPoints_.clear();
    addedPoints_.reserve(cellToMidPoint.size());


    //
    // Introduce cellToMidPoints.
    //

    forAllConstIters(cellToMidPoint, iter)
    {
        const label celli = iter.key();

        label anchorPoint = mesh_.cellPoints()[celli][0];

        label addedPointi =
            meshMod.setAction
            (
                polyAddPoint
                (
                    iter.val(),     // point
                    anchorPoint,    // master point
                    -1,             // zone for point
                    true            // supports a cell
                )
            );
        addedPoints_.insert(celli, addedPointi);

        //Pout<< "Added point " << addedPointi
        //    << iter() << " in cell " << celli << " with centre "
        //    << mesh_.cellCentres()[celli] << endl;
    }


    //
    // Add cells (first one is modified original cell)
    //

    Map<labelList> cellToCells(2*cellToMidPoint.size());

    forAllConstIters(cellToMidPoint, iter)
    {
        const label celli = iter.key();

        const cell& cFaces = mesh_.cells()[celli];

        // Cells created for this cell.
        labelList newCells(cFaces.size());

        // First pyramid is the original cell
        newCells[0] = celli;

        // Add other pyramids
        for (label i = 1; i < cFaces.size(); i++)
        {
            label addedCelli =
                meshMod.setAction
                (
                    polyAddCell
                    (
                        -1,     // master point
                        -1,     // master edge
                        -1,     // master face
                        celli,  // master cell
                        -1      // zone
                    )
                );

            newCells[i] = addedCelli;
        }

        cellToCells.insert(celli, newCells);

        //Pout<< "Split cell " << celli
        //    << " with centre " << mesh_.cellCentres()[celli] << nl
        //    << " faces:" << cFaces << nl
        //    << " into :" << newCells << endl;
    }


    //
    // Introduce internal faces. These go from edges of the cell to the mid
    // point.
    //

    forAllConstIters(cellToMidPoint, iter)
    {
        const label celli = iter.key();

        label midPointi = addedPoints_[celli];

        const cell& cFaces = mesh_.cells()[celli];

        const labelList& cEdges = mesh_.cellEdges()[celli];

        forAll(cEdges, i)
        {
            label edgeI = cEdges[i];
            const edge& e = mesh_.edges()[edgeI];

            // Get the faces on the cell using the edge
            label face0, face1;
            meshTools::getEdgeFaces(mesh_, celli, edgeI, face0, face1);

            // Get the cells on both sides of the face by indexing into cFaces.
            // (since newly created cells are stored in cFaces order)
            const labelList& newCells = cellToCells[celli];

            label cell0 = newCells[cFaces.find(face0)];
            label cell1 = newCells[cFaces.find(face1)];

            if (cell0 < cell1)
            {
                // Construct face to midpoint that is pointing away from
                // (pyramid split off from) celli

                const face& f0 = mesh_.faces()[face0];

                label index = f0.find(e[0]);

                bool edgeInFaceOrder = (f0[f0.fcIndex(index)] == e[1]);

                // Check if celli is the face owner

                face newF(3);
                if (edgeInFaceOrder == (mesh_.faceOwner()[face0] == celli))
                {
                    // edge used in face order.
                    newF[0] = e[1];
                    newF[1] = e[0];
                    newF[2] = midPointi;
                }
                else
                {
                    newF[0] = e[0];
                    newF[1] = e[1];
                    newF[2] = midPointi;
                }

                // Now newF points away from cell0
                meshMod.setAction
                (
                    polyAddFace
                    (
                        newF,                       // face
                        cell0,                      // owner
                        cell1,                      // neighbour
                        -1,                         // master point
                        -1,                         // master edge
                        face0,                      // master face for addition
                        false,                      // flux flip
                        -1,                         // patch for face
                        -1,                         // zone for face
                        false                       // face zone flip
                    )
                );
            }
            else
            {
                // Construct face to midpoint that is pointing away from
                // (pyramid split off from) celli

                const face& f1 = mesh_.faces()[face1];

                label index = f1.find(e[0]);

                bool edgeInFaceOrder = (f1[f1.fcIndex(index)] == e[1]);

                // Check if celli is the face owner

                face newF(3);
                if (edgeInFaceOrder == (mesh_.faceOwner()[face1] == celli))
                {
                    // edge used in face order.
                    newF[0] = e[1];
                    newF[1] = e[0];
                    newF[2] = midPointi;
                }
                else
                {
                    newF[0] = e[0];
                    newF[1] = e[1];
                    newF[2] = midPointi;
                }

                // Now newF points away from cell1
                meshMod.setAction
                (
                    polyAddFace
                    (
                        newF,                       // face
                        cell1,                      // owner
                        cell0,                      // neighbour
                        -1,                         // master point
                        -1,                         // master edge
                        face0,                      // master face for addition
                        false,                      // flux flip
                        -1,                         // patch for face
                        -1,                         // zone for face
                        false                       // face zone flip
                    )
                );
            }
        }
    }


    //
    // Update all existing faces for split owner or neighbour.
    //


    // Mark off affected face.
    bitSet faceUpToDate(mesh_.nFaces(), true);

    forAllConstIters(cellToMidPoint, iter)
    {
        const label celli = iter.key();

        const cell& cFaces = mesh_.cells()[celli];

        faceUpToDate.unset(cFaces);
    }

    forAll(faceUpToDate, facei)
    {
        if (!faceUpToDate.test(facei))
        {
            faceUpToDate.set(facei);

            const face& f = mesh_.faces()[facei];

            if (mesh_.isInternalFace(facei))
            {
                label newOwn = newOwner(facei, cellToCells);
                label newNbr = newNeighbour(facei, cellToCells);

                if (newOwn < newNbr)
                {
                    meshMod.setAction
                    (
                        polyModifyFace
                        (
                            f,
                            facei,
                            newOwn,         // owner
                            newNbr,         // neighbour
                            false,          // flux flip
                            -1,             // patch for face
                            false,          // remove from zone
                            -1,             // zone for face
                            false           // face zone flip
                        )
                    );
                }
                else
                {
                    meshMod.setAction
                    (
                        polyModifyFace
                        (
                            f.reverseFace(),
                            facei,
                            newNbr,         // owner
                            newOwn,         // neighbour
                            false,          // flux flip
                            -1,             // patch for face
                            false,          // remove from zone
                            -1,             // zone for face
                            false           // face zone flip
                        )
                    );
                }

            }
            else
            {
                label newOwn = newOwner(facei, cellToCells);

                label patchID, zoneID, zoneFlip;
                getFaceInfo(facei, patchID, zoneID, zoneFlip);

                meshMod.setAction
                (
                    polyModifyFace
                    (
                        mesh_.faces()[facei],
                        facei,
                        newOwn,         // owner
                        -1,             // neighbour
                        false,          // flux flip
                        patchID,        // patch for face
                        false,          // remove from zone
                        zoneID,         // zone for face
                        zoneFlip        // face zone flip
                    )
                );
            }
        }
    }
}


void Foam::cellSplitter::updateMesh(const mapPolyMesh& morphMap)
{
    // Create copy since we're deleting entries. Only if both cell and added
    // point get mapped do they get inserted.
    Map<label> newAddedPoints(addedPoints_.size());

    forAllConstIters(addedPoints_, iter)
    {
        const label oldCelli = iter.key();
        const label oldPointi = iter.val();

        const label newCelli = morphMap.reverseCellMap()[oldCelli];
        const label newPointi = morphMap.reversePointMap()[oldPointi];

        if (newCelli >= 0 && newPointi >= 0)
        {
            newAddedPoints.insert(newCelli, newPointi);
        }
    }

    // Copy
    addedPoints_.transfer(newAddedPoints);
}


// ************************************************************************* //
