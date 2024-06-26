rho = thermo.rho();

volScalarField rAU(1.0/UEqn.A());
volVectorField HbyA(constrainHbyA(invA & UEqn.H(), U, p));

if (pimple.transonic())
{
    surfaceScalarField phid
    (
        "phid",
        fvc::interpolate(psi)
       *(
            fvc::flux(HbyA)
          + fvc::interpolate(rho*rAU)*fvc::ddtCorr(rho, U, phi)
           /fvc::interpolate(rho)
        )
    );

    while (pimple.correctNonOrthogonal())
    {
        fvScalarMatrix pEqn
        (
            betav*fvm::ddt(psi, p)
          + fvm::div(phid, p)
          - fvm::laplacian(rho*invA, p)
         ==
            betav*fvOptions(psi, p, rho.name())
        );

        pEqn.solve(p.select(pimple.finalInnerIter()));

        if (pimple.finalNonOrthogonalIter())
        {
            phi == pEqn.flux();
        }
    }
}
else
{
    surfaceScalarField phiHbyA
    (
        "phiHbyA",
        (
            fvc::flux(rho*HbyA)
          + fvc::interpolate(rho*rAU)*fvc::ddtCorr(rho, U, phi)
        )
    );

    while (pimple.correctNonOrthogonal())
    {
        fvScalarMatrix pEqn
        (
            betav*fvm::ddt(psi, p)
          + fvc::div(phiHbyA)
          - fvm::laplacian(rho*invA, p)
         ==
            betav*fvOptions(psi, p, rho.name())
        );

        pEqn.solve(p.select(pimple.finalInnerIter()));

        if (pimple.finalNonOrthogonalIter())
        {
            phi = phiHbyA + pEqn.flux();
        }
    }
}

#include "cfdTools/compressible/rhoEqn.H"
#include "cfdTools/incompressible/continuityErrs.H"

U = HbyA - (invA & (betav*fvc::grad(p)));
U.correctBoundaryConditions();
fvOptions.correct(U);
K = 0.5*magSqr(U);

if (thermo.dpdt())
{
    dpdt = fvc::ddt(p);
}
