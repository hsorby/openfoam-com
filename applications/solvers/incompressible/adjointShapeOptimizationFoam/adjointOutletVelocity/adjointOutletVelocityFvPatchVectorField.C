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

\*---------------------------------------------------------------------------*/

#include "adjointOutletVelocity/adjointOutletVelocityFvPatchVectorField.H"
#include "fields/volFields/volFields.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "fields/surfaceFields/surfaceFields.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldMapper.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::adjointOutletVelocityFvPatchVectorField::
adjointOutletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF)
{}


Foam::adjointOutletVelocityFvPatchVectorField::
adjointOutletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF, dict)
{}


Foam::adjointOutletVelocityFvPatchVectorField::
adjointOutletVelocityFvPatchVectorField
(
    const adjointOutletVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper)
{}


Foam::adjointOutletVelocityFvPatchVectorField::
adjointOutletVelocityFvPatchVectorField
(
    const adjointOutletVelocityFvPatchVectorField& pivpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(pivpvf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Update the coefficients associated with the patch field
void Foam::adjointOutletVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const auto& phiap = patch().lookupPatchField<surfaceScalarField>("phia");
    const auto& Up = patch().lookupPatchField<volVectorField>("U");

    scalarField Un(mag(patch().nf() & Up));
    vectorField UtHat((Up - patch().nf()*Un)/(Un + SMALL));

    vectorField Uan(patch().nf()*(patch().nf() & patchInternalField()));

    vectorField::operator=(phiap*patch().Sf()/sqr(patch().magSf()) + UtHat);
    //vectorField::operator=(Uan + UtHat);

    fixedValueFvPatchVectorField::updateCoeffs();
}


void Foam::adjointOutletVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchField<vector>::write(os);
    fvPatchField<vector>::writeValueEntry(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchVectorField,
        adjointOutletVelocityFvPatchVectorField
    );
}


// ************************************************************************* //
