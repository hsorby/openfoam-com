/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2020 OpenCFD Ltd.
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

\*---------------------------------------------------------------------------*/

#include "ODESolvers/adaptiveSolver/adaptiveSolver.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::adaptiveSolver::adaptiveSolver
(
    const ODESystem& ode,
    const dictionary& dict
)
:
    safeScale_(dict.getOrDefault<scalar>("safeScale", 0.9)),
    alphaInc_(dict.getOrDefault<scalar>("alphaIncrease", 0.2)),
    alphaDec_(dict.getOrDefault<scalar>("alphaDecrease", 0.25)),
    minScale_(dict.getOrDefault<scalar>("minScale", 0.2)),
    maxScale_(dict.getOrDefault<scalar>("maxScale", 10)),
    dydx0_(ode.nEqns()),
    yTemp_(ode.nEqns())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::adaptiveSolver::resize(const label n)
{
    ODESolver::resizeField(dydx0_, n);
    ODESolver::resizeField(yTemp_, n);

    return true;
}


void Foam::adaptiveSolver::solve
(
    const ODESystem& odes,
    scalar& x,
    scalarField& y,
    scalar& dxTry
) const
{
    scalar dx = dxTry;
    scalar err = 0.0;

    odes.derivatives(x, y, dydx0_);

    // Loop over solver and adjust step-size as necessary
    // to achieve desired error
    do
    {
        // Solve step and provide error estimate
        err = solve(x, y, dydx0_, dx, yTemp_);

        // If error is large reduce dx
        if (err > 1)
        {
            scalar scale = max(safeScale_*pow(err, -alphaDec_), minScale_);
            dx *= scale;

            if (dx < VSMALL)
            {
                FatalErrorInFunction
                    << "stepsize underflow"
                    << exit(FatalError);
            }
        }
    } while (err > 1);

    // Update the state
    x += dx;
    y = yTemp_;

    // If the error is small increase the step-size
    if (err > pow(maxScale_/safeScale_, -1.0/alphaInc_))
    {
        scalar scale = safeScale_*pow(err, -alphaInc_);
        dxTry = clamp(scale, minScale_, maxScale_)*dx;
    }
    else
    {
        dxTry = safeScale_*maxScale_*dx;
    }
}


// ************************************************************************* //
