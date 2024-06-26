/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
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

InNamespace
    Foam::fvc

Description
    Calculate the first temporal derivative.

SourceFiles
    fvcDdt.C

\*---------------------------------------------------------------------------*/


#ifndef fvcDdt_H
#define fvcDdt_H

#include "fields/volFields/volFieldsFwd.H"
#include "fields/surfaceFields/surfaceFieldsFwd.H"
#include "dimensionedTypes/dimensionedTypes.H"
#include "primitives/traits/one.H"
#include "fields/GeometricFields/geometricZeroField/geometricZeroField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Namespace fvc functions Declaration
\*---------------------------------------------------------------------------*/

namespace fvc
{
    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const dimensioned<Type>,
        const fvMesh&
    );

    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const GeometricField<Type, fvPatchField, volMesh>&
    );

    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const dimensionedScalar&,
        const GeometricField<Type, fvPatchField, volMesh>&
    );

    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const volScalarField&,
        const GeometricField<Type, fvPatchField, volMesh>&
    );

    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const volScalarField&,
        const volScalarField&,
        const GeometricField<Type, fvPatchField, volMesh>&
    );

    template<class Type>
    tmp<GeometricField<Type, fvsPatchField, surfaceMesh>> ddt
    (
        const GeometricField<Type, fvsPatchField, surfaceMesh>&
    );

    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const Foam::one,
        const GeometricField<Type, fvPatchField, volMesh>&
    );

    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh>> ddt
    (
        const GeometricField<Type, fvPatchField, volMesh>&,
        const Foam::one
    );

    inline geometricZeroField ddt
    (
        const Foam::one,
        const Foam::one
    )
    {
        return geometricZeroField();
    }

    template<class Type>
    tmp
    <
        GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >
    >
    ddtCorr
    (
        const GeometricField<Type, fvPatchField, volMesh>& U,
        const GeometricField<Type, fvsPatchField, surfaceMesh>& Uf
    );

    template<class Type>
    tmp
    <
        GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >
    >
    ddtCorr
    (
        const GeometricField<Type, fvPatchField, volMesh>& U,
        const GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >& phi
    );

    template<class Type>
    tmp
    <
        GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >
    >
    ddtCorr
    (
        const GeometricField<Type, fvPatchField, volMesh>& U,
        const GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >& phi,
        const autoPtr<GeometricField<Type, fvsPatchField, surfaceMesh>>& Uf
    );

    template<class Type>
    tmp
    <
        GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >
    >
    ddtCorr
    (
        const volScalarField& rho,
        const GeometricField<Type, fvPatchField, volMesh>& U,
        const GeometricField<Type, fvsPatchField, surfaceMesh>& Uf
    );

    template<class Type>
    tmp
    <
        GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >
    >
    ddtCorr
    (
        const volScalarField& rho,
        const GeometricField<Type, fvPatchField, volMesh>& U,
        const GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >& phi
    );

    template<class Type>
    tmp
    <
        GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >
    >
    ddtCorr
    (
        const volScalarField& rho,
        const GeometricField<Type, fvPatchField, volMesh>& U,
        const GeometricField
        <
            typename Foam::flux<Type>::type,
            fvsPatchField,
            surfaceMesh
        >& phi,
        const autoPtr<GeometricField<Type, fvsPatchField, surfaceMesh>>& Uf
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "finiteVolume/fvc/fvcDdt.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
