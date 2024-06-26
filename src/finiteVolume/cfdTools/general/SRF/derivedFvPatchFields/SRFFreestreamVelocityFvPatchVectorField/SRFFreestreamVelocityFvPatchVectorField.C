/*---------------------------------------------------------------------------* \
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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

#include "cfdTools/general/SRF/derivedFvPatchFields/SRFFreestreamVelocityFvPatchVectorField/SRFFreestreamVelocityFvPatchVectorField.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "fields/volFields/volFields.H"
#include "fields/surfaceFields/surfaceFields.H"
#include "cfdTools/general/SRF/SRFModel/SRFModel/SRFModel.H"
#include "finiteVolume/ddtSchemes/steadyStateDdtScheme/steadyStateDdtScheme.H"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::SRFFreestreamVelocityFvPatchVectorField::
SRFFreestreamVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    inletOutletFvPatchVectorField(p, iF),
    relative_(false),
    UInf_(Zero)
{}


Foam::SRFFreestreamVelocityFvPatchVectorField::
SRFFreestreamVelocityFvPatchVectorField
(
    const SRFFreestreamVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    inletOutletFvPatchVectorField(ptf, p, iF, mapper),
    relative_(ptf.relative_),
    UInf_(ptf.UInf_)
{}


Foam::SRFFreestreamVelocityFvPatchVectorField::
SRFFreestreamVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    inletOutletFvPatchVectorField(p, iF),
    relative_(dict.getOrDefault("relative", false)),
    UInf_(dict.get<vector>("UInf"))
{
    this->phiName_ = dict.getOrDefault<word>("phi", "phi");
    this->readValueEntry(dict, IOobjectOption::MUST_READ);
}


Foam::SRFFreestreamVelocityFvPatchVectorField::
SRFFreestreamVelocityFvPatchVectorField
(
    const SRFFreestreamVelocityFvPatchVectorField& srfvpvf
)
:
    inletOutletFvPatchVectorField(srfvpvf),
    relative_(srfvpvf.relative_),
    UInf_(srfvpvf.UInf_)
{}


Foam::SRFFreestreamVelocityFvPatchVectorField::
SRFFreestreamVelocityFvPatchVectorField
(
    const SRFFreestreamVelocityFvPatchVectorField& srfvpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    inletOutletFvPatchVectorField(srfvpvf, iF),
    relative_(srfvpvf.relative_),
    UInf_(srfvpvf.UInf_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::SRFFreestreamVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    // Get reference to the SRF model
    const SRF::SRFModel& srf =
        db().lookupObject<SRF::SRFModel>("SRFProperties");


    word ddtScheme
    (
        this->internalField().mesh()
       .ddtScheme(this->internalField().name())
    );

    if (ddtScheme == fv::steadyStateDdtScheme<scalar>::typeName)
    {
        // If not relative to the SRF include the effect of the SRF
        if (!relative_)
        {
            refValue() = UInf_ - srf.velocity(patch().Cf());
        }
        // If already relative to the SRF simply supply the inlet value
        // as a fixed value
        else
        {
            refValue() = UInf_;
        }
    }
    else
    {
        scalar time = this->db().time().value();
        scalar theta = time*mag(srf.omega().value());

        refValue() =
            cos(theta)*UInf_ + sin(theta)*(srf.axis() ^ UInf_)
          - srf.velocity(patch().Cf());
    }

    // Set the inlet-outlet choice based on the direction of the freestream
    valueFraction() = neg(refValue() & patch().Sf());

    mixedFvPatchField<vector>::updateCoeffs();
}


void Foam::SRFFreestreamVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchField<vector>::write(os);
    os.writeEntry("relative", relative_);
    os.writeEntry("UInf", UInf_);
    os.writeEntry("phi", this->phiName_);
    fvPatchField<vector>::writeValueEntry(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchVectorField,
        SRFFreestreamVelocityFvPatchVectorField
    );
}


// ************************************************************************* //
