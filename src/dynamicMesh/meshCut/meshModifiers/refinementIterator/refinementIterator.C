/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2019-2022 OpenCFD Ltd.
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

#include "meshCut/meshModifiers/refinementIterator/refinementIterator.H"
#include "meshes/polyMesh/polyMesh.H"
#include "db/Time/TimeOpenFOAM.H"
#include "meshCut/refineCell/refineCell.H"
#include "meshCut/meshModifiers/undoableMeshCutter/undoableMeshCutter.H"
#include "polyTopoChange/polyTopoChange.H"
#include "meshes/polyMesh/mapPolyMesh/mapPolyMesh.H"
#include "meshCut/cellCuts/cellCuts.H"
#include "db/IOstreams/Fstreams/OFstream.H"
#include "meshTools/meshTools.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(refinementIterator, 0);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::refinementIterator::refinementIterator
(
    polyMesh& mesh,
    undoableMeshCutter& meshRefiner,
    const cellLooper& cellWalker,
    const bool writeMesh
)
:
    edgeVertex(mesh),
    mesh_(mesh),
    meshRefiner_(meshRefiner),
    cellWalker_(cellWalker),
    writeMesh_(writeMesh)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::refinementIterator::~refinementIterator()
{}  // Define here since polyMesh was forward declared


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::Map<Foam::label> Foam::refinementIterator::setRefinement
(
    const List<refineCell>& refCells
)
{
    Map<label> addedCells(2*refCells.size());

    Time& runTime = const_cast<Time&>(mesh_.time());

    label nRefCells = refCells.size();

    label oldRefCells = -1;

    // Operate on copy.
    List<refineCell> currentRefCells(refCells);

    bool stop = false;

    do
    {
        if (writeMesh_)
        {
            // Need different times to write meshes.
            ++runTime;
        }

        polyTopoChange meshMod(mesh_);

        if (debug)
        {
            Pout<< "refinementIterator : refining "
                << currentRefCells.size() << " cells." << endl;
        }

        // Determine cut pattern.
        cellCuts cuts(mesh_, cellWalker_, currentRefCells);

        if (!returnReduceOr(cuts.nLoops()))
        {
            if (debug)
            {
                Pout<< "refinementIterator : exiting iteration since no valid"
                    << " loops found for " << currentRefCells.size()
                    << " cells" << endl;


                fileName cutsFile("failedCuts_" + runTime.timeName() + ".obj");

                Pout<< "Writing cuts for time " <<  runTime.timeName()
                    << " to " << cutsFile << endl;

                OFstream cutsStream(cutsFile);


                labelList refCellsDebug(currentRefCells.size());
                forAll(currentRefCells, i)
                {
                    refCellsDebug[i] = currentRefCells[i].cellNo();
                }
                meshTools::writeOBJ
                (
                    cutsStream,
                    mesh().cells(),
                    mesh().faces(),
                    mesh().points(),
                    refCellsDebug
                );
            }

            break;
        }

        if (debug)
        {
            fileName cutsFile("cuts_" + runTime.timeName() + ".obj");

            Pout<< "Writing cuts for time " <<  runTime.timeName()
                << " to " << cutsFile << endl;

            OFstream cutsStream(cutsFile);
            cuts.writeOBJ(cutsStream);
        }


        // Insert mesh refinement into polyTopoChange.
        meshRefiner_.setRefinement(cuts, meshMod);


        //
        // Do all changes
        //

        autoPtr<mapPolyMesh> morphMap = meshMod.changeMesh
        (
            mesh_,
            false
        );

        // Move mesh (since morphing does not do this)
        if (morphMap().hasMotionPoints())
        {
            mesh_.movePoints(morphMap().preMotionPoints());
        }

        // Update stored refinement pattern
        meshRefiner_.updateMesh(morphMap());

        // Write resulting mesh
        if (writeMesh_)
        {
            if (debug)
            {
                Pout<< "Writing refined polyMesh to time "
                    << runTime.timeName() << endl;
            }

            mesh_.write();
        }

        // Update currentRefCells for new cell numbers. Use helper function
        // in meshCutter class.
        updateLabels
        (
            morphMap->reverseCellMap(),
            currentRefCells
        );

        // Update addedCells for new cell numbers
        updateLabels
        (
            morphMap->reverseCellMap(),
            addedCells
        );

        // Get all added cells from cellCutter (already in new numbering
        // from meshRefiner.updateMesh call) and add to global list of added
        const Map<label>& addedNow = meshRefiner_.addedCells();

        forAllConstIters(addedNow, iter)
        {
            if (!addedCells.insert(iter.key(), iter.val()))
            {
                FatalErrorInFunction
                    << "Master cell " << iter.key()
                    << " already has been refined" << endl
                    << "Added cell:" << iter.val() << abort(FatalError);
            }
        }


        // Get failed refinement in new cell numbering and reconstruct input
        // to the meshRefiner. Is done by removing all refined cells from
        // current list of cells to refine.

        // Update refCells for new cell numbers.
        updateLabels
        (
            morphMap->reverseCellMap(),
            currentRefCells
        );

        // Pack refCells acc. to refined status
        nRefCells = 0;

        forAll(currentRefCells, refI)
        {
            const refineCell& refCell = currentRefCells[refI];

            if (!addedNow.found(refCell.cellNo()))
            {
                if (nRefCells != refI)
                {
                    currentRefCells[nRefCells++] =
                        refineCell
                        (
                            refCell.cellNo(),
                            refCell.direction()
                        );
                }
            }
        }

        oldRefCells = currentRefCells.size();

        currentRefCells.setSize(nRefCells);

        if (debug)
        {
            Pout<< endl;
        }

        // Stop only if all finished or all can't refine any further.
        stop = returnReduceAnd(nRefCells == 0 || nRefCells == oldRefCells);
    }
    while (!stop);


    if (returnReduceAnd(nRefCells == oldRefCells))
    {
        WarningInFunction
            << "stopped refining."
            << "Did not manage to refine a single cell" << endl
            << "Wanted :" << oldRefCells << endl;
    }

    return addedCells;
}



// ************************************************************************* //
