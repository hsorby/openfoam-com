/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2012 OpenFOAM Foundation
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

#include "fields/fvPatchFields/fvPatchField/fvPatchFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{

#define makeFvPatchField(PatchTypeField)                                      \
    defineTemplateRunTimeSelectionTable(PatchTypeField, patch);               \
    defineTemplateRunTimeSelectionTable(PatchTypeField, patchMapper);         \
    defineTemplateRunTimeSelectionTable(PatchTypeField, dictionary);

makeFvPatchField(fvPatchScalarField);
makeFvPatchField(fvPatchVectorField);
makeFvPatchField(fvPatchSphericalTensorField);
makeFvPatchField(fvPatchSymmTensorField);
makeFvPatchField(fvPatchTensorField);

} // End namespace Foam

// ************************************************************************* //
