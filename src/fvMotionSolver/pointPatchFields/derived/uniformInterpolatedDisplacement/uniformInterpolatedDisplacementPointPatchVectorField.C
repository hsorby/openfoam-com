/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2016 OpenFOAM Foundation
    Copyright (C) 2019-2020,2022 OpenCFD Ltd.
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

#include "pointPatchFields/derived/uniformInterpolatedDisplacement/uniformInterpolatedDisplacementPointPatchVectorField.H"
#include "fields/GeometricFields/pointFields/pointFields.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "db/Time/TimeOpenFOAM.H"
#include "meshes/polyMesh/polyMesh.H"
#include "interpolations/interpolationWeights/interpolationWeights/interpolationWeights.H"
#include "fields/GeometricFields/GeometricField/uniformInterpolate.H"
#include "fields/ReadFields/ReadFieldsPascal.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

uniformInterpolatedDisplacementPointPatchVectorField::
uniformInterpolatedDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(p, iF)
{}


uniformInterpolatedDisplacementPointPatchVectorField::
uniformInterpolatedDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const dictionary& dict
)
:
    fixedValuePointPatchField<vector>(p, iF, dict),
    fieldName_(dict.lookup("field")),
    interpolationScheme_(dict.lookup("interpolationScheme"))
{
    if (!dict.found("value"))
    {
        updateCoeffs();
    }
}


uniformInterpolatedDisplacementPointPatchVectorField::
uniformInterpolatedDisplacementPointPatchVectorField
(
    const uniformInterpolatedDisplacementPointPatchVectorField& ptf,
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    fixedValuePointPatchField<vector>(ptf, p, iF, mapper),
    fieldName_(ptf.fieldName_),
    interpolationScheme_(ptf.interpolationScheme_),
    timeNames_(ptf.timeNames_),
    timeVals_(ptf.timeVals_),
    interpolatorPtr_(ptf.interpolatorPtr_)
{}


uniformInterpolatedDisplacementPointPatchVectorField::
uniformInterpolatedDisplacementPointPatchVectorField
(
    const uniformInterpolatedDisplacementPointPatchVectorField& ptf,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(ptf, iF),
    fieldName_(ptf.fieldName_),
    interpolationScheme_(ptf.interpolationScheme_),
    timeNames_(ptf.timeNames_),
    timeVals_(ptf.timeVals_),
    interpolatorPtr_(ptf.interpolatorPtr_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void uniformInterpolatedDisplacementPointPatchVectorField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    if (!interpolatorPtr_)
    {
        const pointMesh& pMesh = this->internalField().mesh();

        // Read time values
        const instantList allTimes = Time::findTimes(pMesh().time().path());

        // Only keep those that contain the field
        DynamicList<word> names(allTimes.size());
        DynamicList<scalar> values(allTimes.size());

        for (const instant& inst : allTimes)
        {
            IOobject io
            (
                fieldName_,
                inst.name(),
                pMesh(),
                IOobject::MUST_READ,
                IOobject::NO_WRITE,
                IOobject::NO_REGISTER
            );
            if (io.typeHeaderOk<pointVectorField>(false))
            {
                names.append(inst.name());
                values.append(inst.value());
            }
        }
        timeNames_.transfer(names);
        timeVals_.transfer(values);

        Info<< type() << " : found " << fieldName_ << " for times "
            << timeNames_ << endl;

        if (timeNames_.size() < 1)
        {
            FatalErrorInFunction
                << "Did not find any times with " << fieldName_
                << exit(FatalError);
        }


        interpolatorPtr_ = interpolationWeights::New
        (
            interpolationScheme_,
            timeVals_
        );
    }

    const pointMesh& pMesh = this->internalField().mesh();
    const Time& t = pMesh().time();

    // Update indices of times and weights
    bool timesChanged = interpolatorPtr_->valueWeights
    (
        t.timeOutputValue(),
        currentIndices_,
        currentWeights_
    );

    const wordList currentTimeNames
    (
        UIndirectList<word>(timeNames_, currentIndices_)
    );


    // Load if necessary fields for this interpolation
    if (timesChanged)
    {
        objectRegistry& fieldsCache = const_cast<objectRegistry&>
        (
            pMesh.thisDb().subRegistry("fieldsCache", true)
        );
        // Save old times so we know which ones have been loaded and need
        // 'correctBoundaryConditions'. Bit messy.
        wordHashSet oldTimes(fieldsCache.toc());

        ReadFields<pointVectorField>
        (
            fieldName_,
            pMesh,
            currentTimeNames
        );

        forAllConstIters(fieldsCache, fieldsCacheIter)
        {
            if (!oldTimes.found(fieldsCacheIter.key()))
            {
                // Newly loaded fields. Make sure the internal
                // values are consistent with the boundary conditions.
                // This is quite often not the case since these
                // fields typically are constructed 'by hand'

                const objectRegistry& timeCache = dynamic_cast
                <
                    const objectRegistry&
                >(*fieldsCacheIter());


                pointVectorField& d =
                    timeCache.lookupObjectRef<pointVectorField>(fieldName_);

                d.correctBoundaryConditions();
            }
        }
    }


    // Interpolate the whole field
    pointVectorField result
    (
        uniformInterpolate<pointVectorField>
        (
            IOobject
            (
                word("uniformInterpolate(")
              + this->internalField().name()
              + ')',
                pMesh.time().timeName(),
                pMesh.thisDb(),
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            fieldName_,
            currentTimeNames,
            currentWeights_
        )
    );


    // Extract back from the internal field
    this->operator==
    (
        this->patchInternalField(result())
    );

    fixedValuePointPatchField<vector>::updateCoeffs();
}


void uniformInterpolatedDisplacementPointPatchVectorField::write(Ostream& os)
const
{
    pointPatchField<vector>::write(os);
    os.writeEntry("field", fieldName_);
    os.writeEntry("interpolationScheme", interpolationScheme_);
    this->writeValueEntry(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePointPatchTypeField
(
    pointPatchVectorField,
    uniformInterpolatedDisplacementPointPatchVectorField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
