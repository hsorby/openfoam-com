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
    magneticFoam

Group
    grpElectroMagneticsSolvers

Description
    Solver for the magnetic field generated by permanent magnets.

    A Poisson's equation for the magnetic scalar potential psi is solved
    from which the magnetic field intensity H and magnetic flux density B
    are obtained.  The paramagnetic particle force field (H dot grad(H))
    is optionally available.

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "include/OSspecific.H"
#include "magnet.H"
#include "global/constants/electromagnetic/electromagneticConstants.H"
#include "cfdTools/general/solutionControl/simpleControl/simpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Solver for the magnetic field generated by permanent magnets."
    );

    argList::addBoolOption
    (
        "noH",
        "Do not write the magnetic field intensity field"
    );

    argList::addBoolOption
    (
        "noB",
        "Do not write the magnetic flux density field"
    );

    argList::addBoolOption
    (
        "HdotGradH",
        "Write the paramagnetic particle force field"
    );

    #include "include/addCheckCaseOptions.H"
    #include "include/setRootCaseLists.H"
    #include "include/createTime.H"
    #include "include/createMesh.H"

    simpleControl simple(mesh);

    #include "createFields.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "Calculating the magnetic field potential" << endl;

    ++runTime;

    while (simple.correctNonOrthogonal())
    {
        solve(fvm::laplacian(murf, psi) + fvc::div(murf*Mrf));
    }

    psi.write();

    if (!args.found("noH") || args.found("HdotGradH"))
    {
        volVectorField H
        (
            IOobject
            (
                "H",
                runTime.timeName(),
                mesh
            ),
            fvc::reconstruct(fvc::snGrad(psi)*mesh.magSf())
        );

        if (!args.found("noH"))
        {
            Info<< nl
                << "Creating field H for time "
                << runTime.timeName() << endl;

            H.write();
        }

        if (args.found("HdotGradH"))
        {
            Info<< nl
                << "Creating field HdotGradH for time "
                << runTime.timeName() << endl;

            volVectorField HdotGradH
            (
                IOobject
                (
                    "HdotGradH",
                    runTime.timeName(),
                    mesh
                ),
                H & fvc::grad(H)
            );

            HdotGradH.write();
        }
    }

    if (!args.found("noB"))
    {
        Info<< nl
            << "Creating field B for time "
            << runTime.timeName() << endl;

        volVectorField B
        (
            IOobject
            (
                "B",
                runTime.timeName(),
                mesh
            ),
            constant::electromagnetic::mu0
           *fvc::reconstruct(murf*fvc::snGrad(psi)*mesh.magSf() + murf*Mrf)
        );

        B.write();
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
