/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016-2017 Wikki Ltd
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

#include "finiteArea/convectionSchemes/gaussFaConvectionScheme/gaussFaConvectionScheme.H"
#include "finiteArea/fac/facEdgeIntegrate.H"
#include "faMatrices/faMatrices.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace fa
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
tmp<GeometricField<Type, faePatchField, edgeMesh>>
gaussConvectionScheme<Type>::flux
(
    const edgeScalarField& faceFlux,
    const GeometricField<Type, faPatchField, areaMesh>& vf
) const
{
    return faceFlux*tinterpScheme_().interpolate(vf);
}


template<class Type>
tmp<faMatrix<Type>>
gaussConvectionScheme<Type>::famDiv
(
    const edgeScalarField& faceFlux,
    const GeometricField<Type, faPatchField, areaMesh>& vf
) const
{
    tmp<edgeScalarField> tweights = tinterpScheme_().weights(vf);
    const edgeScalarField& weights = tweights();

    tmp<faMatrix<Type>> tfam
    (
        new faMatrix<Type>
        (
            vf,
            faceFlux.dimensions()*vf.dimensions()
        )
    );
    faMatrix<Type>& fam = tfam.ref();

    fam.lower() = -weights.internalField()*faceFlux.internalField();
    fam.upper() = fam.lower() + faceFlux.internalField();
    fam.negSumDiag();

    forAll(fam.psi().boundaryField(), patchI)
    {
        const faPatchField<Type>& psf = fam.psi().boundaryField()[patchI];
        const faePatchScalarField& patchFlux = faceFlux.boundaryField()[patchI];
        const faePatchScalarField& pw = weights.boundaryField()[patchI];

        fam.internalCoeffs()[patchI] = patchFlux*psf.valueInternalCoeffs(pw);
        fam.boundaryCoeffs()[patchI] = -patchFlux*psf.valueBoundaryCoeffs(pw);
    }

//     if (tinterpScheme_().corrected())
//     {
//         fam += fac::edgeIntegrate(faceFlux*tinterpScheme_().correction(vf));
//     }

    // Non-euclidian and other corrections
    GeometricField<Type, faePatchField, edgeMesh> convFluxCorr
    (
        flux(faceFlux, vf)
      - faceFlux*tinterpScheme_().euclidianInterpolate(vf)
    );

    fam += fac::edgeIntegrate(convFluxCorr);

    return tfam;
}


template<class Type>
tmp<GeometricField<Type, faPatchField, areaMesh>>
gaussConvectionScheme<Type>::facDiv
(
    const edgeScalarField& faceFlux,
    const GeometricField<Type, faPatchField, areaMesh>& vf
) const
{
    tmp<GeometricField<Type, faPatchField, areaMesh>> tConvection
    (
        fac::edgeIntegrate(flux(faceFlux, vf))
    );

    tConvection.ref().rename
    (
        "convection(" + faceFlux.name() + ',' + vf.name() + ')'
    );

    return tConvection;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fa

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
