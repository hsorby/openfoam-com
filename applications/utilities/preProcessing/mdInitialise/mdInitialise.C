/*---------------------------------------------------------------------------*\
 =========                   |
 \\      /   F ield          | OpenFOAM: The Open Source CFD Toolbox
  \\    /    O peration      |
   \\  /     A nd            |
    \\/      M anipulation   |
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
    mdInitialise

Group
    grpPreProcessingUtilities

Description
    Initialises fields for a molecular dynamics (MD) simulation.

\*---------------------------------------------------------------------------*/

#include "mdTools/md.H"
#include "cfdTools/general/include/fvCFD.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Initialises fields for a molecular dynamics (MD) simulation"
    );
    #include "include/setRootCase.H"
    #include "include/createTime.H"
    #include "include/createNamedMesh.H"

    IOdictionary mdInitialiseDict
    (
        IOobject
        (
            "mdInitialiseDict",
            runTime.system(),
            runTime,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE,
            IOobject::NO_REGISTER
        )
    );

    IOdictionary idListDict
    (
        IOobject
        (
            "idList",
            mesh.time().constant(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        )
    );

    potential pot(mesh, mdInitialiseDict, idListDict);

    moleculeCloud molecules(mesh, pot, mdInitialiseDict);

    label totalMolecules = molecules.size();

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
            << "Failed writing moleculeCloud."
            << nl << exit(FatalError);
    }

    Info<< nl;
    runTime.printExecutionTime(Info);

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
