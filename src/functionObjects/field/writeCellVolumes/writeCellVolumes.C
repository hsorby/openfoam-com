/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016 OpenFOAM Foundation
    Copyright (C) 2016-2020 OpenCFD Ltd.
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

#include "writeCellVolumes/writeCellVolumes.H"
#include "fields/volFields/volFields.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(writeCellVolumes, 0);
    addToRunTimeSelectionTable(functionObject, writeCellVolumes, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::writeCellVolumes::writeCellVolumes
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict)
{
    read(dict);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::writeCellVolumes::read(const dictionary& dict)
{
    return fvMeshFunctionObject::read(dict);
}


bool Foam::functionObjects::writeCellVolumes::execute()
{
    return true;
}


bool Foam::functionObjects::writeCellVolumes::write()
{
    volScalarField V
    (
        IOobject
        (
            mesh_.V().name(),
            time_.timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            IOobject::NO_REGISTER
        ),
        mesh_,
        dimensionedScalar(mesh_.V().dimensions(), Zero),
        fvPatchFieldBase::calculatedType()
    );

    V.ref() = mesh_.V();

    // Without correctBoundaryConditions() - boundary faces have zero volume

    Log << type() << " " << name() << " write:" << nl
        << "    writing cell-volumes field " << V.name()
        << " to " << time_.timeName() << endl;

    V.write();

    return true;
}


// ************************************************************************* //
