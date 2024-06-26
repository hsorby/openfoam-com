/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2013-2016 OpenFOAM Foundation
    Copyright (C) 2018-2020 OpenCFD Ltd.
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

#include "fields/fvPatchFields/derived/prghPressure/prghPressureFvPatchScalarField.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldMapper.H"
#include "fields/volFields/volFields.H"
#include "cfdTools/general/meshObjects/gravity/gravityMeshObject.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::prghPressureFvPatchScalarField::
prghPressureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    rhoName_("rho"),
    p_(p.size(), Zero)
{}


Foam::prghPressureFvPatchScalarField::
prghPressureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF, dict, IOobjectOption::NO_READ),
    rhoName_(dict.getOrDefault<word>("rho", "rho")),
    p_("p", dict, p.size())
{
    if (!this->readValueEntry(dict))
    {
        fvPatchField<scalar>::operator=(p_);
    }
}


Foam::prghPressureFvPatchScalarField::
prghPressureFvPatchScalarField
(
    const prghPressureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    rhoName_(ptf.rhoName_),
    p_(ptf.p_, mapper)
{}


Foam::prghPressureFvPatchScalarField::
prghPressureFvPatchScalarField
(
    const prghPressureFvPatchScalarField& ptf
)
:
    fixedValueFvPatchScalarField(ptf),
    rhoName_(ptf.rhoName_),
    p_(ptf.p_)
{}


Foam::prghPressureFvPatchScalarField::
prghPressureFvPatchScalarField
(
    const prghPressureFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(ptf, iF),
    rhoName_(ptf.rhoName_),
    p_(ptf.p_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::prghPressureFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedValueFvPatchScalarField::autoMap(m);
    p_.autoMap(m);
}


void Foam::prghPressureFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    fixedValueFvPatchScalarField::rmap(ptf, addr);

    const prghPressureFvPatchScalarField& tiptf =
        refCast<const prghPressureFvPatchScalarField>(ptf);

    p_.rmap(tiptf.p_, addr);
}


void Foam::prghPressureFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const scalarField& rhop =
        patch().lookupPatchField<volScalarField>(rhoName_);

    const uniformDimensionedVectorField& g =
        meshObjects::gravity::New(db().time());

    const uniformDimensionedScalarField& hRef =
        db().lookupObject<uniformDimensionedScalarField>("hRef");

    dimensionedScalar ghRef
    (
        mag(g.value()) > SMALL
      ? g & (cmptMag(g.value())/mag(g.value()))*hRef
      : dimensionedScalar("ghRef", g.dimensions()*dimLength, 0)
    );

    operator==(p_ - rhop*((g.value() & patch().Cf()) - ghRef.value()));

    fixedValueFvPatchScalarField::updateCoeffs();
}


void Foam::prghPressureFvPatchScalarField::write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    os.writeEntryIfDifferent<word>("rho", "rho", rhoName_);
    p_.writeEntry("p", os);
    fvPatchField<scalar>::writeValueEntry(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        prghPressureFvPatchScalarField
    );
}

// ************************************************************************* //
