/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2017 OpenFOAM Foundation
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

#include "derivedFvPatchFields/semiPermeableBaffle/semiPermeableBaffleVelocity/semiPermeableBaffleVelocityFvPatchVectorField.H"
#include "derivedFvPatchFields/semiPermeableBaffle/semiPermeableBaffleMassFraction/semiPermeableBaffleMassFractionFvPatchScalarField.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldMapper.H"
#include "fields/volFields/volFields.H"
#include "fields/surfaceFields/surfaceFields.H"
#include "psiReactionThermo/psiReactionThermo.H"
#include "rhoReactionThermo/rhoReactionThermo.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

const Foam::basicSpecieMixture&
Foam::semiPermeableBaffleVelocityFvPatchVectorField::composition() const
{
    const word& name = basicThermo::dictName;

    if (db().foundObject<psiReactionThermo>(name))
    {
        return db().lookupObject<psiReactionThermo>(name).composition();
    }
    else if (db().foundObject<rhoReactionThermo>(name))
    {
        return db().lookupObject<rhoReactionThermo>(name).composition();
    }
    else
    {
        FatalErrorInFunction
            << "Could not find a multi-component thermodynamic model."
            << exit(FatalError);

        return NullObjectRef<basicSpecieMixture>();
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::semiPermeableBaffleVelocityFvPatchVectorField::
semiPermeableBaffleVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF),
    rhoName_("rho")
{}


Foam::semiPermeableBaffleVelocityFvPatchVectorField::
semiPermeableBaffleVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF),
    rhoName_(dict.getOrDefault<word>("rho", "rho"))
{
    this->readValueEntry(dict, IOobjectOption::MUST_READ);
}


Foam::semiPermeableBaffleVelocityFvPatchVectorField::
semiPermeableBaffleVelocityFvPatchVectorField
(
    const semiPermeableBaffleVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
    rhoName_(ptf.rhoName_)
{}


Foam::semiPermeableBaffleVelocityFvPatchVectorField::
semiPermeableBaffleVelocityFvPatchVectorField
(
    const semiPermeableBaffleVelocityFvPatchVectorField& ptf
)
:
    fixedValueFvPatchVectorField(ptf),
    rhoName_(ptf.rhoName_)
{}


Foam::semiPermeableBaffleVelocityFvPatchVectorField::
semiPermeableBaffleVelocityFvPatchVectorField
(
    const semiPermeableBaffleVelocityFvPatchVectorField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(ptf, iF),
    rhoName_(ptf.rhoName_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::semiPermeableBaffleVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    typedef semiPermeableBaffleMassFractionFvPatchScalarField YBCType;

    const scalarField& rhop =
        patch().lookupPatchField<volScalarField>(rhoName_);

    const PtrList<volScalarField>& Y = composition().Y();

    scalarField phip(patch().size(), Zero);
    forAll(Y, i)
    {
        const fvPatchScalarField& Yp = Y[i].boundaryField()[patch().index()];

        if (!isA<YBCType>(Yp))
        {
            FatalErrorInFunction
                << "The mass-fraction condition on patch " << patch().name()
                << " is not of type " << YBCType::typeName << "."
                << exit(FatalError);
        }

        phip += refCast<const YBCType>(Yp).phiY();
    }

    this->operator==(patch().nf()*phip/(rhop*patch().magSf()));

    fixedValueFvPatchVectorField::updateCoeffs();
}


void Foam::semiPermeableBaffleVelocityFvPatchVectorField::write
(
    Ostream& os
) const
{
    fvPatchField<vector>::write(os);
    os.writeEntryIfDifferent<word>("rho", "rho", rhoName_);
    fvPatchField<vector>::writeValueEntry(os);
}


// * * * * * * * * * * * * * * Build Macro Function  * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchVectorField,
        semiPermeableBaffleVelocityFvPatchVectorField
    );
}

// ************************************************************************* //
