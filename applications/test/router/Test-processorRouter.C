/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2020 OpenCFD Ltd.
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

Description

\*---------------------------------------------------------------------------*/

#include "global/argList/argList.H"
#include "primitives/ints/label/label.H"
#include "primitives/ints/lists/labelList.H"
#include "db/IOstreams/Fstreams/Fstream.H"
#include "meshes/primitiveShapes/point/point.H"
#include "db/Time/TimeOpenFOAM.H"
#include "fvMesh/fvMesh.H"
#include "router.H"
#include "meshes/polyMesh/polyPatches/constraint/processor/processorPolyPatch.H"
#include "db/typeInfo/typeInfo.H"
#include "Gather/Gather.H"


using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Get list of my processor neighbours
labelList procNeighbours(const polyMesh& mesh)
{
    word procLabel = '[' + word(name(Pstream::myProcNo())) + "]-";

    label nNeighbours = 0;

    forAll(mesh.boundaryMesh(), patchi)
    {
        if (isA<processorPolyPatch>(mesh.boundaryMesh()[patchi]))
        {
            ++nNeighbours;
        }
    }

    labelList neighbours(nNeighbours);

    nNeighbours = 0;

    forAll(mesh.boundaryMesh(), patchi)
    {
        const auto* procPatch =
            isA<processorPolyPatch>(mesh.boundaryMesh()[patchi]);

        if (procPatch)
        {
            neighbours[nNeighbours++] = procPatch->neighbProcNo();
        }
    }

    return neighbours;
}


// Calculate some average position for mesh.
point meshCentre(const polyMesh& mesh)
{
    return average(mesh.points());
}


// Main program:


int main(int argc, char *argv[])
{
    #include "include/setRootCase.H"
    #include "include/createTime.H"
    #include "include/createPolyMesh.H"

    word procLabel = '[' + word(name(Pstream::myProcNo())) + "]-";

    if (!Pstream::parRun())
    {
        FatalErrorInFunction
            << "Please run in parallel" << exit(FatalError);
    }

    // 'Gather' processor-processor topology
    Gather<labelList> connections
    (
        static_cast<const labelList&>(procNeighbours(mesh))
    );

    // Collect centres of individual meshes (for visualization only)
    Gather<point> meshCentres(meshCentre(mesh));

    if (!Pstream::master())
    {
        return 0;
    }


    //
    // At this point we have the connections between processors and the
    // processor-mesh centres.
    //

    Info<< "connections:" << connections << endl;
    Info<< "meshCentres:" << meshCentres << endl;


    //
    // Dump connections and meshCentres to OBJ file
    //

    fileName fName("decomposition.obj");

    Info<< "Writing decomposition to " << fName << endl;

    OFstream objFile(fName);

    // Write processors as single vertex in centre of mesh
    forAll(meshCentres, proci)
    {
        const point& pt = meshCentres[proci];

        objFile << "v " << pt.x() << ' ' << pt.y() << ' ' << pt.z() << endl;
    }
    // Write connections as lines between processors (duplicated)
    forAll(connections, proci)
    {
        const labelList& nbs = connections[proci];

        forAll(nbs, nbI)
        {
            objFile << "l " << proci + 1 << ' ' << nbs[nbI] + 1 << endl;
        }
    }


    //
    // Read paths to route from dictionary
    //

    IFstream dictFile("routerDict");

    dictionary routeDict(dictFile);

    labelListList paths(routeDict.lookup("paths"));



    //
    // Iterate over routing. Route as much as possible during each iteration
    // and stop if all paths have been routed. No special ordering to maximize
    // routing during one iteration.
    //

    boolList routeOk(paths.size(), false);
    label nOk = 0;

    label iter = 0;

    while (nOk < paths.size())
    {
        Info<< "Iteration:" << iter << endl;
        Info<< "---------------" << endl;


        // Dump to OBJ file

        fileName fName("route_" + name(iter) + ".obj");
        Info<< "Writing route to " << fName << endl;

        OFstream objFile(fName);

        forAll(meshCentres, proci)
        {
            const point& pt = meshCentres[proci];

            objFile << "v " << pt.x() << ' ' << pt.y() << ' ' << pt.z()
                    << endl;
        }


        // Router
        router cellRouter(connections, meshCentres);

        // Try to route as many paths possible during this iteration.
        forAll(paths, pathI)
        {
            if (!routeOk[pathI])
            {
                const labelList& path = paths[pathI];

                Info<< "Trying to route path " << pathI
                    << " nodes " << path << endl;

                routeOk[pathI] = cellRouter.route(path, -(pathI + 1));

                Info<< "Result of routing:" << routeOk[pathI] << endl;

                if (routeOk[pathI])
                {
                    nOk++;

                    // Dump route as lines.
                    labelList route(cellRouter.getRoute(-(pathI + 1)));

                    for (label elemI = 1; elemI < route.size(); elemI++)
                    {
                        objFile
                            << "l " << route[elemI-1]+1 << ' '
                            << route[elemI]+1 << endl;
                    }
                    Info<< "route:" << route << endl;
                }
            }
        }
        Info<< endl;

        iter++;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
