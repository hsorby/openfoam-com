/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2015 OpenFOAM Foundation
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
    dsmcInitialise

Group
    grpPreProcessingUtilities

Description
    Initialise a case for dsmcFoam by reading the initialisation dictionary
    system/dsmcInitialise.

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "clouds/derived/dsmcCloud/dsmcCloud.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Initialise a case for dsmcFoam from the system/dsmcInitialise"
        " dictionary"
    );

    #include "include/setRootCase.H"
    #include "include/createTime.H"
    #include "include/createNamedMesh.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    IOdictionary dsmcInitialiseDict
    (
        IOobject
        (
            "dsmcInitialiseDict",
            mesh.time().system(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    );

    Info<< "Initialising dsmc for Time = " << runTime.timeName() << nl << endl;

    dsmcCloud dsmc("dsmc", mesh, dsmcInitialiseDict);

    label totalMolecules = dsmc.size();

    if (Pstream::parRun())
    {
        reduce(totalMolecules, sumOp<label>());
    }

    Info<< nl << "Total number of molecules added: " << totalMolecules
        << nl << endl;

    IOstream::defaultPrecision(15);

    if (!mesh.write())
    {
        FatalErrorInFunction
            << "Failed writing dsmcCloud."
            << nl << exit(FatalError);
    }

    Info<< nl;
    runTime.printExecutionTime(Info);

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
