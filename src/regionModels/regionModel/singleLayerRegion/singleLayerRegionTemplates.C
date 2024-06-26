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

#include "singleLayerRegion/singleLayerRegion.H"
#include "fields/fvPatchFields/basic/zeroGradient/zeroGradientFvPatchFields.H"
#include "fields/fvPatchFields/derived/mappedFixedInternalValue/mappedFixedInternalValueFvPatchFields.H"
#include "fields/fvPatchFields/derived/mappedFixedPushedInternalValue/mappedFixedPushedInternalValueFvPatchFields.H"

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

template<class Type>
Foam::wordList
Foam::regionModels::singleLayerRegion::mappedFieldAndInternalPatchTypes() const
{
    wordList bTypes
    (
        regionMesh().boundaryMesh().size(),
        fvPatchFieldBase::zeroGradientType()
    );

    for (const label patchi : intCoupledPatchIDs_)
    {
        bTypes[patchi] = mappedFixedInternalValueFvPatchField<Type>::typeName;
    }

    return bTypes;
}


template<class Type>
Foam::wordList
Foam::regionModels::singleLayerRegion::mappedPushedFieldPatchTypes() const
{
    wordList bTypes
    (
        regionMesh().boundaryMesh().size(),
        fvPatchFieldBase::zeroGradientType()
    );

    for (const label patchi : intCoupledPatchIDs_)
    {
        bTypes[patchi] =
            mappedFixedPushedInternalValueFvPatchField<Type>::typeName;
    }

    return bTypes;
}


// ************************************************************************* //
