/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016-2017 Wikki Ltd
    Copyright (C) 2021-2023 OpenCFD Ltd.
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

Application
    makeFaMesh

Description
    A mesh generator for finiteArea mesh.
    When called in parallel, it will also try to act like decomposePar,
    create procAddressing and decompose serial finite-area fields.

Original Authors
    Zeljko Tukovic, FAMENA
    Hrvoje Jasak, Wikki Ltd.

\*---------------------------------------------------------------------------*/

#include "db/Time/TimeOpenFOAM.H"
#include "global/argList/argList.H"
#include "include/OSspecific.H"
#include "faMesh/faMesh.H"
#include "faMesh/faMeshTools/faMeshTools.H"
#include "db/IOobjects/IOdictionary/IOdictionary.H"
#include "db/IOobjectList/IOobjectList.H"
#include "fields/areaFields/areaFields.H"
#include "fields/edgeFields/edgeFields.H"
#include "faFieldDecomposer.H"
#include "faMeshReconstructor.H"
#include "faMesh/faMeshSubset/faMeshSubset.H"
#include "containers/PtrLists/PtrListOps/PtrListOps.H"
#include "vtk/write/foamVtkLineWriter.H"
#include "output/vtk/patch/foamVtkIndPatchWriter.H"
#include "meshes/polyMesh/syncTools/syncTools.H"
#include "obj/OBJstream.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "A mesh generator for finiteArea mesh"
    );
    argList::addOption
    (
        "empty-patch",
        "name",
        "Specify name for a default empty patch",
        false  // An advanced option, but not enough to worry about that
    );
    argList::addOption("dict", "file", "Alternative faMeshDefinition");

    argList::addDryRunOption
    (
        "Create but do not write"
    );
    argList::addBoolOption
    (
        "no-decompose",
        "Suppress procAddressing creation and field decomposition"
        " (parallel)"
    );
    argList::addBoolOption
    (
        "no-fields",
        "Suppress field decomposition"
        " (parallel)"
    );
    argList::addBoolOption
    (
        "write-vtk",
        "Write mesh as a vtp (vtk) file for display or debugging"
    );
    argList::addBoolOption
    (
        "write-edges-obj",
        "Write mesh edges as obj files (one per processor)",
        true  // advanced option (debugging only)
    );

    #include "include/addRegionOption.H"
    #include "include/setRootCase.H"
    #include "include/createTime.H"
    #include "include/createNamedPolyMesh.H"

    const bool doDecompose = !args.found("no-decompose");
    const bool doDecompFields = !args.found("no-fields");

    if (!doDecompose)
    {
        Info<< "Skip decompose of finiteArea mesh/fields" << nl;
    }
    else if (!doDecompFields)
    {
        Info<< "Skip decompose of finiteArea fields" << nl;
    }

    // Reading faMeshDefinition dictionary
    #include "findMeshDefinitionDict.H"

    // Inject/overwrite name for optional 'empty' patch
    word patchName;
    if (args.readIfPresent("empty-patch", patchName))
    {
        meshDefDict.add("emptyPatch", patchName, true);
    }

    // Preliminary checks
    #include "checkPatchTopology.H"

    // Create
    faMesh aMesh(mesh, meshDefDict);

    // Mesh information (less verbose)
    faMeshTools::printMeshChecks(aMesh, 0);

    if (args.found("write-edges-obj"))
    {
        #include "faMeshWriteEdgesOBJ.H"
    }

    if (args.found("write-vtk"))
    {
        #include "faMeshWriteVTK.H"
    }

    if (args.dryRun())
    {
        Info<< "\ndry-run: not writing mesh or decomposing fields\n" << nl;
    }
    else
    {
        // Set the precision of the points data to 10
        IOstream::defaultPrecision(10);

        Info<< nl << "Write finite area mesh." << nl;
        aMesh.write();

        Info<< endl;
        #include "decomposeFaFields.H"
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}

// ************************************************************************* //
