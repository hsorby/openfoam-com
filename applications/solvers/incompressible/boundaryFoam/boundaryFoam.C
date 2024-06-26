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
    boundaryFoam

Group
    grpIncompressibleSolvers

Description
    Steady-state solver for incompressible, 1D turbulent flow,
    typically to generate boundary layer conditions at an inlet.

    Boundary layer code to calculate the U, k and epsilon distributions.
    Used to create inlet boundary conditions for experimental comparisons
    for which U and k have not been measured.
    Turbulence model is runtime selectable.

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "singlePhaseTransportModel/singlePhaseTransportModel.H"
#include "turbulentTransportModels/turbulentTransportModel.H"
#include "cfdTools/general/fvOptions/fvOptions.H"
#include "fvMesh/fvPatches/derived/wall/wallFvPatch.H"
#include "graphField/makeGraph.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Steady-state solver for incompressible, 1D turbulent flow,"
        " typically to generate boundary layer conditions at an inlet."
    );

    argList::noParallel();
    #include "include/addCheckCaseOptions.H"

    #include "include/setRootCaseLists.H"
    #include "include/createTime.H"
    #include "include/createMesh.H"
    #include "createFields.H"
    #include "interrogateWallPatches.H"

    turbulence->validate();

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        fvVectorMatrix divR(turbulence->divDevReff(U));
        divR.source() = flowMask & divR.source();

        fvVectorMatrix UEqn
        (
            divR == gradP + fvOptions(U)
        );

        UEqn.relax();

        fvOptions.constrain(UEqn);

        UEqn.solve();

        fvOptions.correct(U);


        // Correct driving force for a constant volume flow rate
        dimensionedVector UbarStar = flowMask & U.weightedAverage(mesh.V());

        U += (Ubar - UbarStar);
        gradP += (Ubar - UbarStar)/(1.0/UEqn.A())().weightedAverage(mesh.V());

        laminarTransport.correct();
        turbulence->correct();

        Info<< "Uncorrected Ubar = " << (flowDirection & UbarStar.value())
            << ", pressure gradient = " << (flowDirection & gradP.value())
            << endl;

        #include "evaluateNearWall.H"

        if (runTime.writeTime())
        {
            #include "makeGraphs.H"
        }

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
