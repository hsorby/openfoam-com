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

#include "fields/fvPatchFields/derived/advective/advectiveFvPatchField.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldMapper.H"
#include "fields/volFields/volFields.H"
#include "finiteVolume/ddtSchemes/EulerDdtScheme/EulerDdtScheme.H"
#include "finiteVolume/ddtSchemes/CrankNicolsonDdtScheme/CrankNicolsonDdtScheme.H"
#include "finiteVolume/ddtSchemes/backwardDdtScheme/backwardDdtScheme.H"
#include "finiteVolume/ddtSchemes/localEulerDdtScheme/localEulerDdtScheme.H"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::advectiveFvPatchField<Type>::advectiveFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    mixedFvPatchField<Type>(p, iF),
    phiName_("phi"),
    rhoName_("rho"),
    fieldInf_(Zero),
    lInf_(-GREAT)
{
    this->refValue() = Zero;
    this->refGrad() = Zero;
    this->valueFraction() = 0.0;
}


template<class Type>
Foam::advectiveFvPatchField<Type>::advectiveFvPatchField
(
    const advectiveFvPatchField& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    mixedFvPatchField<Type>(ptf, p, iF, mapper),
    phiName_(ptf.phiName_),
    rhoName_(ptf.rhoName_),
    fieldInf_(ptf.fieldInf_),
    lInf_(ptf.lInf_)
{}


template<class Type>
Foam::advectiveFvPatchField<Type>::advectiveFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchField<Type>(p, iF),
    phiName_(dict.getOrDefault<word>("phi", "phi")),
    rhoName_(dict.getOrDefault<word>("rho", "rho")),
    fieldInf_(Zero),
    lInf_(-GREAT)
{
    // Use 'value' supplied, or set to internal field
    if (!this->readValueEntry(dict))
    {
        this->extrapolateInternal();  // Zero-gradient patch values
    }

    this->refValue() = *this;
    this->refGrad() = Zero;
    this->valueFraction() = 0;

    if (dict.readIfPresent("lInf", lInf_))
    {
        dict.readEntry("fieldInf", fieldInf_);

        if (lInf_ < 0.0)
        {
            FatalIOErrorInFunction(dict)
                << "unphysical lInf specified (lInf < 0)" << nl
                << "    on patch " << this->patch().name()
                << " of field " << this->internalField().name()
                << " in file " << this->internalField().objectPath()
                << exit(FatalIOError);
        }
    }
}


template<class Type>
Foam::advectiveFvPatchField<Type>::advectiveFvPatchField
(
    const advectiveFvPatchField& ptpsf
)
:
    mixedFvPatchField<Type>(ptpsf),
    phiName_(ptpsf.phiName_),
    rhoName_(ptpsf.rhoName_),
    fieldInf_(ptpsf.fieldInf_),
    lInf_(ptpsf.lInf_)
{}


template<class Type>
Foam::advectiveFvPatchField<Type>::advectiveFvPatchField
(
    const advectiveFvPatchField& ptpsf,
    const DimensionedField<Type, volMesh>& iF
)
:
    mixedFvPatchField<Type>(ptpsf, iF),
    phiName_(ptpsf.phiName_),
    rhoName_(ptpsf.rhoName_),
    fieldInf_(ptpsf.fieldInf_),
    lInf_(ptpsf.lInf_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::scalarField>
Foam::advectiveFvPatchField<Type>::advectionSpeed() const
{
    const auto& phip =
        this->patch().template lookupPatchField<surfaceScalarField>(phiName_);

    if (phip.internalField().dimensions() == dimMass/dimTime)
    {
        const auto& rhop =
            this->patch().template lookupPatchField<volScalarField>(rhoName_);

        return phip/(rhop*this->patch().magSf());
    }
    else
    {
        return phip/this->patch().magSf();
    }
}


template<class Type>
void Foam::advectiveFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    const fvMesh& mesh = this->internalField().mesh();

    word ddtScheme
    (
        mesh.ddtScheme(this->internalField().name())
    );
    scalar deltaT = this->db().time().deltaTValue();

    const GeometricField<Type, fvPatchField, volMesh>& field =
        this->db().objectRegistry::template
        lookupObject<GeometricField<Type, fvPatchField, volMesh>>
        (
            this->internalField().name()
        );

    // Calculate the advection speed of the field wave
    // If the wave is incoming set the speed to 0.
    const scalarField w(Foam::max(advectionSpeed(), scalar(0)));

    // Calculate the field wave coefficient alpha (See notes)
    const scalarField alpha(w*deltaT*this->patch().deltaCoeffs());

    label patchi = this->patch().index();

    // Non-reflecting outflow boundary
    // If lInf_ defined setup relaxation to the value fieldInf_.
    if (lInf_ > 0)
    {
        // Calculate the field relaxation coefficient k (See notes)
        const scalarField k(w*deltaT/lInf_);

        if
        (
            ddtScheme == fv::EulerDdtScheme<scalar>::typeName
         || ddtScheme == fv::CrankNicolsonDdtScheme<scalar>::typeName
        )
        {
            this->refValue() =
            (
                field.oldTime().boundaryField()[patchi] + k*fieldInf_
            )/(1.0 + k);

            this->valueFraction() = (1.0 + k)/(1.0 + alpha + k);
        }
        else if (ddtScheme == fv::backwardDdtScheme<scalar>::typeName)
        {
            this->refValue() =
            (
                2.0*field.oldTime().boundaryField()[patchi]
              - 0.5*field.oldTime().oldTime().boundaryField()[patchi]
              + k*fieldInf_
            )/(1.5 + k);

            this->valueFraction() = (1.5 + k)/(1.5 + alpha + k);
        }
        else if
        (
            ddtScheme == fv::localEulerDdtScheme<scalar>::typeName
        )
        {
            const volScalarField& rDeltaT =
                fv::localEulerDdt::localRDeltaT(mesh);

            // Calculate the field wave coefficient alpha (See notes)
            const scalarField alpha
            (
                w*this->patch().deltaCoeffs()/rDeltaT.boundaryField()[patchi]
            );

            // Calculate the field relaxation coefficient k (See notes)
            const scalarField k(w/(rDeltaT.boundaryField()[patchi]*lInf_));

            this->refValue() =
            (
                field.oldTime().boundaryField()[patchi] + k*fieldInf_
            )/(1.0 + k);

            this->valueFraction() = (1.0 + k)/(1.0 + alpha + k);
        }
        else
        {
            FatalErrorInFunction
                << ddtScheme << nl
                << "    on patch " << this->patch().name()
                << " of field " << this->internalField().name()
                << " in file " << this->internalField().objectPath()
                << exit(FatalError);
        }
    }
    else
    {
        if
        (
            ddtScheme == fv::EulerDdtScheme<scalar>::typeName
         || ddtScheme == fv::CrankNicolsonDdtScheme<scalar>::typeName
        )
        {
            this->refValue() = field.oldTime().boundaryField()[patchi];

            this->valueFraction() = 1.0/(1.0 + alpha);
        }
        else if (ddtScheme == fv::backwardDdtScheme<scalar>::typeName)
        {
            this->refValue() =
            (
                2.0*field.oldTime().boundaryField()[patchi]
              - 0.5*field.oldTime().oldTime().boundaryField()[patchi]
            )/1.5;

            this->valueFraction() = 1.5/(1.5 + alpha);
        }
        else if
        (
            ddtScheme == fv::localEulerDdtScheme<scalar>::typeName
        )
        {
            const volScalarField& rDeltaT =
                fv::localEulerDdt::localRDeltaT(mesh);

            // Calculate the field wave coefficient alpha (See notes)
            const scalarField alpha
            (
                w*this->patch().deltaCoeffs()/rDeltaT.boundaryField()[patchi]
            );

            this->refValue() = field.oldTime().boundaryField()[patchi];

            this->valueFraction() = 1.0/(1.0 + alpha);
        }
        else
        {
            FatalErrorInFunction
                << ddtScheme
                << "\n    on patch " << this->patch().name()
                << " of field " << this->internalField().name()
                << " in file " << this->internalField().objectPath()
                << exit(FatalError);
        }
    }

    mixedFvPatchField<Type>::updateCoeffs();
}


template<class Type>
void Foam::advectiveFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);

    os.writeEntryIfDifferent<word>("phi", "phi", phiName_);
    os.writeEntryIfDifferent<word>("rho", "rho", rhoName_);

    if (lInf_ > 0)
    {
        os.writeEntry("fieldInf", fieldInf_);
        os.writeEntry("lInf", lInf_);
    }

    fvPatchField<Type>::writeValueEntry(os);
}


// ************************************************************************* //
