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

Group
    grpCompressibleSolvers

Application
    sonicLiquidFoam

Description
    Transient solver for trans-sonic/supersonic, laminar flow of a
    compressible liquid.

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "cfdTools/general/solutionControl/pimpleControl/pimpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Transient solver for trans-sonic/supersonic, laminar flow"
        " of a compressible liquid."
    );

    #include "db/functionObjects/functionObjectList/postProcess.H"

    #include "include/addCheckCaseOptions.H"
    #include "include/setRootCaseLists.H"
    #include "include/createTime.H"
    #include "include/createMesh.H"
    #include "cfdTools/general/solutionControl/createControl.H"
    #include "createFields.H"
    #include "fluid/initContinuityErrs.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        #include "fluid/compressibleCourantNo.H"

        solve(fvm::ddt(rho) + fvc::div(phi));

        // --- Pressure-velocity PIMPLE corrector loop
        while (pimple.loop())
        {
            fvVectorMatrix UEqn
            (
                fvm::ddt(rho, U)
              + fvm::div(phi, U)
              - fvm::laplacian(mu, U)
            );

            solve(UEqn == -fvc::grad(p));

            // --- Pressure corrector loop
            while (pimple.correct())
            {
                volScalarField rAU("rAU", 1.0/UEqn.A());
                surfaceScalarField rhorAUf
                (
                    "rhorAUf",
                    fvc::interpolate(rho*rAU)
                );

                U = rAU*UEqn.H();

                surfaceScalarField phid
                (
                    "phid",
                    psi
                   *(
                       fvc::flux(U)
                     + rhorAUf*fvc::ddtCorr(rho, U, phi)/fvc::interpolate(rho)
                    )
                );

                phi = (rhoO/psi)*phid;

                fvScalarMatrix pEqn
                (
                    fvm::ddt(psi, p)
                  + fvc::div(phi)
                  + fvm::div(phid, p)
                  - fvm::laplacian(rhorAUf, p)
                );

                pEqn.solve();

                phi += pEqn.flux();

                solve(fvm::ddt(rho) + fvc::div(phi));
                #include "cfdTools/compressible/compressibleContinuityErrs.H"

                U -= rAU*fvc::grad(p);
                U.correctBoundaryConditions();
            }
        }

        rho = rhoO + psi*p;

        runTime.write();

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
