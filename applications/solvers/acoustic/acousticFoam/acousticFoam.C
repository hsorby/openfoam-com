/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2019-2020 OpenCFD Ltd.
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
   acousticFoam

Group
    grpAcousticSolvers

Description
    Acoustic solver solving the acoustic pressure wave equation.

    \f[
        \ddt2{pa} - c^2 \laplacian{pa} = 0
    \f]

    where
    \vartable
        c       | Sound speed
        pa      | Acoustic pressure
    \endvartable

SourceFiles
    acousticFoam.C

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "cfdTools/general/fvOptions/fvOptions.H"
#include "cfdTools/general/solutionControl/pimpleControl/pimpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Acoustic solver solving the acoustic pressure wave equation."
    );

    #include "db/functionObjects/functionObjectList/postProcess.H"

    #include "include/addCheckCaseOptions.H"
    #include "include/setRootCaseLists.H"
    #include "include/createTime.H"
    #include "include/createMesh.H"
    #include "cfdTools/general/solutionControl/createControl.H"
    #include "createRegionControls.H"

    #include "readTransportProperties.H"
    #include "createFields.H"

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.run())
    {
        ++runTime;

        Info<< "Time = " << runTime.timeName() << nl << endl;

        while (pimple.correct())
        {
            #include "paEqn.H"
        }

        runTime.write();

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
