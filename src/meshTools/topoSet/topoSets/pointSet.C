/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2016-2022 OpenCFD Ltd.
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

#include "topoSet/topoSets/pointSet.H"
#include "meshes/polyMesh/mapPolyMesh/mapPolyMesh.H"
#include "meshes/polyMesh/polyMesh.H"
#include "meshes/polyMesh/syncTools/syncTools.H"
#include "meshes/polyMesh/mapPolyMesh/mapDistribute/mapDistributePolyMesh.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(pointSet, 0);
    addToRunTimeSelectionTable(topoSet, pointSet, word);
    addToRunTimeSelectionTable(topoSet, pointSet, size);
    addToRunTimeSelectionTable(topoSet, pointSet, set);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::pointSet::pointSet(const IOobject& io)
:
    topoSet(io, typeName)
{}


Foam::pointSet::pointSet
(
    const polyMesh& mesh,
    const word& name,
    IOobjectOption::readOption rOpt,
    IOobjectOption::writeOption wOpt
)
:
    topoSet(mesh, typeName, name, rOpt, wOpt)
{
    check(mesh.nPoints());
}


Foam::pointSet::pointSet
(
    const polyMesh& mesh,
    const word& name,
    const label size,
    IOobjectOption::writeOption wOpt
)
:
    topoSet(mesh, name, size, wOpt)
{}


Foam::pointSet::pointSet
(
    const polyMesh& mesh,
    const word& name,
    const topoSet& set,
    IOobjectOption::writeOption wOpt
)
:
    topoSet(mesh, name, set, wOpt)
{}


Foam::pointSet::pointSet
(
    const polyMesh& mesh,
    const word& name,
    const labelHashSet& labels,
    IOobjectOption::writeOption wOpt
)
:
    topoSet(mesh, name, labels, wOpt)
{}


Foam::pointSet::pointSet
(
    const polyMesh& mesh,
    const word& name,
    labelHashSet&& labels,
    IOobjectOption::writeOption wOpt
)
:
    topoSet(mesh, name, std::move(labels), wOpt)
{}


Foam::pointSet::pointSet
(
    const polyMesh& mesh,
    const word& name,
    const labelUList& labels,
    IOobjectOption::writeOption wOpt
)
:
    topoSet(mesh, name, labels, wOpt)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::pointSet::sync(const polyMesh& mesh)
{
    labelHashSet& labels = *this;

    // Convert to boolList
    // TBD: could change to using bitSet for the synchronization

    const label len = mesh.nPoints();

    boolList contents(len, false);

    for (const label pointi : labels)
    {
        contents.set(pointi);
    }

    // The nullValue = 'false'
    syncTools::syncPointList(mesh, contents, orEqOp<bool>(), false);


    // Update labelHashSet

    for (label pointi=0; pointi < len; ++pointi)
    {
        if (contents.test(pointi))
        {
            labels.set(pointi);
        }
    }
}


Foam::label Foam::pointSet::maxSize(const polyMesh& mesh) const
{
    return mesh.nPoints();
}


void Foam::pointSet::updateMesh(const mapPolyMesh& morphMap)
{
    updateLabels(morphMap.reversePointMap());
}


void Foam::pointSet::distribute(const mapDistributePolyMesh& map)
{
    labelHashSet& labels = *this;

    boolList contents(map.nOldPoints(), false);

    for (const label pointi : labels)
    {
        contents.set(pointi);
    }

    map.distributePointData(contents);

    // The new length
    const label len = contents.size();

    // Count - as per BitOps::count(contents)
    label n = 0;
    for (label i=0; i < len; ++i)
    {
        if (contents.test(i))
        {
            ++n;
        }
    }


    // Update labelHashSet

    labels.clear();
    labels.reserve(n);

    for (label i=0; i < len; ++i)
    {
        if (contents.test(i))
        {
            labels.set(i);
        }
    }
}


void Foam::pointSet::writeDebug
(
    Ostream& os,
    const primitiveMesh& mesh,
    const label maxLen
) const
{
    topoSet::writeDebug(os, mesh.points(), maxLen);
}


// ************************************************************************* //
