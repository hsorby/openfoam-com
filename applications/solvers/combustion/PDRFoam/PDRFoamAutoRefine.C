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

Application
    PDRFoam

Description
    Solver for compressible premixed/partially-premixed combustion with
    turbulence modelling.

    Combusting RANS code using the b-Xi two-equation model.
    Xi may be obtained by either the solution of the Xi transport
    equation or from an algebraic expression.  Both approaches are
    based on Gulder's flame speed correlation which has been shown
    to be appropriate by comparison with the results from the
    spectral model.

    Strain effects are incorporated directly into the Xi equation
    but not in the algebraic approximation.  Further work need to be
    done on this issue, particularly regarding the enhanced removal rate
    caused by flame compression.  Analysis using results of the spectral
    model will be required.

    For cases involving very lean Propane flames or other flames which are
    very strain-sensitive, a transport equation for the laminar flame
    speed is present.  This equation is derived using heuristic arguments
    involving the strain time scale and the strain-rate at extinction.
    the transport velocity is the same as that for the Xi equation.

    For large flames e.g. explosions additional modelling for the flame
    wrinkling due to surface instabilities may be applied.

    PDR (porosity/distributed resistance) modelling is included to handle
    regions containing blockages which cannot be resolved by the mesh.

\*---------------------------------------------------------------------------*/

#include "cfdTools/general/include/fvCFD.H"
#include "dynamicFvMesh/dynamicFvMesh.H"
#include "psiuReactionThermo/psiuReactionThermo.H"
#include "turbulentFluidThermoModels/turbulentFluidThermoModel.H"
#include "laminarFlameSpeed/laminarFlameSpeed.H"
#include "XiModels/XiModel/XiModel.H"
#include "PDRModels/dragModels/PDRDragModel/PDRDragModel.H"
#include "ignition/ignition.H"
#include "cfdTools/general/bound/bound.H"
#include "dynamicRefineFvMesh/dynamicRefineFvMesh.H"
#include "cfdTools/general/solutionControl/pimpleControl/pimpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Solver for compressible premixed/partially-premixed combustion with"
        " turbulence modelling."
    );

    #include "include/setRootCaseLists.H"
    #include "include/createTime.H"
    #include "include/createDynamicFvMesh.H"

    pimpleControl pimple(mesh);

    #include "readCombustionProperties.H"
    #include "cfdTools/general/include/readGravitationalAcceleration.H"
    #include "createFields.H"
    #include "fluid/initContinuityErrs.H"
    #include "cfdTools/general/include/createTimeControls.H"
    #include "fluid/compressibleCourantNo.H"
    #include "cfdTools/general/include/setInitialDeltaT.H"

    turbulence->validate();
    scalar StCoNum = 0.0;

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    bool hasChanged = false;

    while (runTime.run())
    {
        #include "cfdTools/general/include/readTimeControls.H"
        #include "fluid/compressibleCourantNo.H"
        #include "cfdTools/general/include/setDeltaT.H"

        // Indicators for refinement.
        // Note: before ++runTime only for post-processing reasons.
        tmp<volScalarField> tmagGradP = mag(fvc::grad(p));
        volScalarField normalisedGradP
        (
            "normalisedGradP",
            tmagGradP()/max(tmagGradP())
        );
        normalisedGradP.writeOpt(IOobject::AUTO_WRITE);
        tmagGradP.clear();

        ++runTime;

        Info<< "\n\nTime = " << runTime.timeName() << endl;

        {
            // Make the fluxes absolute
            fvc::makeAbsolute(phi, rho, U);

            // Test : disable refinement for some cells
            bitSet& protectedCell =
                refCast<dynamicRefineFvMesh>(mesh).protectedCell();

            if (protectedCell.empty())
            {
                protectedCell.setSize(mesh.nCells());
                protectedCell = false;
            }

            forAll(betav, celli)
            {
                if (betav[celli] < 0.99)
                {
                    protectedCell.set(celli);
                }
            }

            // Flux estimate for introduced faces.
            volVectorField rhoU("rhoU", rho*U);

            // Do any mesh changes
            bool meshChanged = mesh.update();


            if (meshChanged)
            {
                hasChanged = true;
            }

            if (runTime.write() && hasChanged)
            {
                betav.write();
                Lobs.write();
                CT.write();
                drag->writeFields();
                flameWrinkling->writeFields();
                hasChanged = false;
            }

            // Make the fluxes relative to the mesh motion
            fvc::makeRelative(phi, rho, U);
        }


        #include "cfdTools/compressible/rhoEqn.H"

        // --- Pressure-velocity PIMPLE corrector loop
        while (pimple.loop())
        {
            #include "fluid/UEqn.H"


            // --- Pressure corrector loop
            while (pimple.correct())
            {
                #include "bEqn.H"
                #include "ftEqn.H"
                #include "huEqn.H"
                #include "hEqn.H"

                if (!ign.ignited())
                {
                    hu == h;
                }

                #include "fluid/pEqn.H"
            }

            if (pimple.turbCorr())
            {
                turbulence->correct();
            }
        }

        runTime.write();

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n";

    return 0;
}


// ************************************************************************* //
