/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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
    chemFoam

Group
    grpCombustionSolvers

Description
    Solver for chemistry problems, designed for use on single cell cases to
    provide comparison against other chemistry solvers, that uses a single cell
    mesh, and fields created from the initial conditions.

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "psiReactionThermo/psiReactionThermo.H"
#include "BasicChemistryModel.H"
#include "mixtures/reactingMixture/reactingMixture.H"
#include "chemistrySolver/chemistrySolver/chemistrySolver.H"
#include "db/IOstreams/Fstreams/OFstream.H"
#include "include/thermoPhysicsTypes.H"
#include "mixtures/basicSpecieMixture/basicSpecieMixture.H"
#include "fvMesh/simplifiedFvMesh/hexCellFvMesh/hexCellFvMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Solver for chemistry problems, designed for use on single cell cases"
        " to provide comparison against other chemistry solvers"
    );

    argList::noParallel();

    #define CREATE_MESH createSingleCellMesh.H
    #define NO_CONTROL
    #include "db/functionObjects/functionObjectList/postProcess.H"

    #include "include/setRootCaseLists.H"
    #include "include/createTime.H"
    #include "createSingleCellMesh.H"
    #include "createFields.H"
    #include "createFieldRefs.H"
    #include "readInitialConditions.H"
    #include "include/createControls.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.run())
    {
        #include "readControls.H"

        #include "cfdTools/general/include/setDeltaT.H"

        ++runTime;
        Info<< "Time = " << runTime.timeName() << nl << endl;

        #include "solveChemistry.H"
        #include "fluid/YEqn.H"
        #include "hEqn.H"
        #include "fluid/pEqn.H"

        #include "output.H"

        runTime.printExecutionTime(Info);
    }

    Info<< "Number of steps = " << runTime.timeIndex() << nl;
    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
