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

#include "sources/interRegion/interRegionHeatTransfer/tabulatedHeatTransfer/tabulatedHeatTransfer.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{
    defineTypeNameAndDebug(tabulatedHeatTransfer, 0);
    addToRunTimeSelectionTable(option, tabulatedHeatTransfer, dictionary);
}
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

const Foam::interpolation2DTable<Foam::scalar>&
Foam::fv::tabulatedHeatTransfer::hTable()
{
    if (!hTable_)
    {
        hTable_.reset(new interpolation2DTable<scalar>(coeffs_));
    }

    return *hTable_;
}


const Foam::volScalarField& Foam::fv::tabulatedHeatTransfer::AoV()
{
    if (!AoV_)
    {
        AoV_.reset
        (
            new volScalarField
            (
                IOobject
                (
                    "AoV",
                    startTimeName_,
                    mesh_,
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE
                ),
                mesh_
            )
        );
    }

    return *AoV_;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::tabulatedHeatTransfer::tabulatedHeatTransfer
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    interRegionHeatTransferModel(name, modelType, dict, mesh),
    UName_(coeffs_.getOrDefault<word>("U", "U")),
    UNbrName_(coeffs_.getOrDefault<word>("UNbr", "U")),
    hTable_(),
    AoV_(),
    startTimeName_(mesh.time().timeName())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fv::tabulatedHeatTransfer::calculateHtc()
{
    const auto& nbrMesh = mesh_.time().lookupObject<fvMesh>(nbrRegionName());

    const auto& UNbr = nbrMesh.lookupObject<volVectorField>(UNbrName_);

    const scalarField UMagNbr(mag(UNbr));

    const scalarField UMagNbrMapped(interpolate(UMagNbr));

    const auto& U = mesh_.lookupObject<volVectorField>(UName_);

    scalarField& htcc = htc_.primitiveFieldRef();

    forAll(htcc, i)
    {
        htcc[i] = hTable()(mag(U[i]), UMagNbrMapped[i]);
    }

    htcc = htcc*AoV();
}


bool Foam::fv::tabulatedHeatTransfer::read(const dictionary& dict)
{
    if (interRegionHeatTransferModel::read(dict))
    {
        return true;
    }

    return false;
}


// ************************************************************************* //
