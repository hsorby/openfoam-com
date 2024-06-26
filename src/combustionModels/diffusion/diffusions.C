/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2017 OpenFOAM Foundation
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

#include "combustionModel/makeCombustionTypes.H"

#include "include/thermoPhysicsTypes.H"
#include "psiReactionThermo/psiReactionThermo.H"
#include "rhoReactionThermo/rhoReactionThermo.H"
#include "diffusion/diffusion.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Combustion models based on sensibleEnthalpy
makeCombustionTypesThermo
(
    diffusion,
    psiReactionThermo,
    gasHThermoPhysics
);

makeCombustionTypesThermo
(
    diffusion,
    psiReactionThermo,
    constGasHThermoPhysics
);

makeCombustionTypesThermo
(
    diffusion,
    rhoReactionThermo,
    gasHThermoPhysics
);

makeCombustionTypesThermo
(
    diffusion,
    rhoReactionThermo,
    constGasHThermoPhysics
);


// Combustion models based on sensibleInternalEnergy

makeCombustionTypesThermo
(
    diffusion,
    psiReactionThermo,
    gasEThermoPhysics
);

makeCombustionTypesThermo
(
    diffusion,
    psiReactionThermo,
    constGasEThermoPhysics
);

makeCombustionTypesThermo
(
    diffusion,
    rhoReactionThermo,
    gasEThermoPhysics
);

makeCombustionTypesThermo
(
    diffusion,
    rhoReactionThermo,
    constGasEThermoPhysics
);

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
