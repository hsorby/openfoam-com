/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2019 OpenCFD Ltd.
    Copyright (C) 2019-2020 DLR
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

#include "implicitFunctions/composedFunction/composedFunctionImplicitFunction.H"
#include "fields/Fields/scalarField/scalarField.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    namespace implicitFunctions
    {
        defineTypeNameAndDebug(composedFunctionImplicitFunction, 0);
        addToRunTimeSelectionTable
        (
            implicitFunction,
            composedFunctionImplicitFunction,
            dict
        );
    }
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

const Foam::Enum
<
    Foam::implicitFunctions::composedFunctionImplicitFunction::modeType
>
Foam::implicitFunctions::composedFunctionImplicitFunction::modeTypeNames
({
    { modeType::ADD, "add" },
    { modeType::SUBTRACT, "subtract" },
    { modeType::MINDIST, "minDist" },
    { modeType::INTERSECT, "intersect" },
});


Foam::label
Foam::implicitFunctions::composedFunctionImplicitFunction::selectFunction
(
    const scalarField& values
) const
{
    switch (mode_)
    {
        case modeType::MINDIST:
        {
            scalarField absVal(mag(values));
            return findMin(absVal);
        }
        case modeType::ADD:
        {
            return findMax(values);
        }
        case modeType::SUBTRACT:
        {
            // Note: start at the second entry
            const label idx = findMin(values, 1);

            if (values[idx] < values[0] && pos(values[0]))
            {
                return idx;
            }
            else
            {
                return 0;
            }
        }
        case modeType::INTERSECT:
        {
            return findMin(values);
        }
        default:
        {
            FatalErrorInFunction
                << "This mode is not supported  only " << nl
                << "Supported modes are: " << nl
                << modeTypeNames
                << abort(FatalError);
        }
    }

    return -1;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::implicitFunctions::composedFunctionImplicitFunction::
composedFunctionImplicitFunction
(
    const dictionary& dict
)
:
    functions_(),
    mode_(modeTypeNames.get("mode", dict)),
    values_()
{
    const dictionary& funcDict = dict.subDict("composedFunction");

    functions_.resize(funcDict.size());
    values_.resize(funcDict.size(), Zero);

    label funci = 0;

    for (const entry& dEntry : funcDict)
    {
        const word& key = dEntry.keyword();

        if (!dEntry.isDict())
        {
            FatalIOErrorInFunction(funcDict)
                << "Entry " << key << " is not a dictionary" << endl
                << exit(FatalIOError);
        }

        const dictionary& subdict = dEntry.dict();

        functions_.set
        (
            funci,
            implicitFunction::New(subdict.get<word>("type"), subdict)
        );

        ++funci;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::scalar Foam::implicitFunctions::composedFunctionImplicitFunction::value
(
    const vector& p
) const
{
    forAll(values_,i)
    {
        values_[i] = functions_[i].value(p);
    }

    const label idx = selectFunction(values_);

    return values_[idx];
}


Foam::vector Foam::implicitFunctions::composedFunctionImplicitFunction::grad
(
    const vector& p
) const
{
    forAll(values_,i)
    {
        values_[i] = mag(functions_[i].value(p));
    }

    const label minIdx = findMin(values_);

    return functions_[minIdx].grad(p);
}


Foam::scalar
Foam::implicitFunctions::composedFunctionImplicitFunction::distanceToSurfaces
(
    const vector& p
) const
{
    forAll(values_,i)
    {
        values_[i] = mag(functions_[i].value(p));
    }

    const label minIdx = findMin(values_);

    return functions_[minIdx].distanceToSurfaces(p);
}


// ************************************************************************* //
