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

\*---------------------------------------------------------------------------*/

#include "threePhaseInterfaceProperties/threePhaseInterfaceProperties.H"
#include "alphaContactAngle/alphaContactAngleTwoPhaseFvPatchScalarField.H"
#include "global/constants/mathematical/mathematicalConstants.H"
#include "interpolation/surfaceInterpolation/surfaceInterpolation/surfaceInterpolate.H"
#include "finiteVolume/fvc/fvcDiv.H"
#include "finiteVolume/fvc/fvcGrad.H"
#include "finiteVolume/fvc/fvcSnGrad.H"
#include "global/constants/unitConversion.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::threePhaseInterfaceProperties::correctContactAngle
(
    surfaceVectorField::Boundary& nHatb
) const
{
    const volScalarField::Boundary& alpha1 =
        mixture_.alpha1().boundaryField();
    const volScalarField::Boundary& alpha2 =
        mixture_.alpha2().boundaryField();
    const volScalarField::Boundary& alpha3 =
        mixture_.alpha3().boundaryField();
    const volVectorField::Boundary& U =
        mixture_.U().boundaryField();

    const fvMesh& mesh = mixture_.U().mesh();
    const fvBoundaryMesh& boundary = mesh.boundary();

    forAll(boundary, patchi)
    {
        if (isA<alphaContactAngleTwoPhaseFvPatchScalarField>(alpha1[patchi]))
        {
            const alphaContactAngleTwoPhaseFvPatchScalarField& a2cap =
                refCast<const alphaContactAngleTwoPhaseFvPatchScalarField>
                (alpha2[patchi]);

            const alphaContactAngleTwoPhaseFvPatchScalarField& a3cap =
                refCast<const alphaContactAngleTwoPhaseFvPatchScalarField>
                (alpha3[patchi]);

            scalarField twoPhaseAlpha2(max(a2cap, scalar(0)));
            scalarField twoPhaseAlpha3(max(a3cap, scalar(0)));

            scalarField sumTwoPhaseAlpha
            (
                twoPhaseAlpha2 + twoPhaseAlpha3 + SMALL
            );

            twoPhaseAlpha2 /= sumTwoPhaseAlpha;
            twoPhaseAlpha3 /= sumTwoPhaseAlpha;

            fvsPatchVectorField& nHatp = nHatb[patchi];

            scalarField theta
            (
                degToRad()
              * (
                   twoPhaseAlpha2*(180 - a2cap.theta(U[patchi], nHatp))
                 + twoPhaseAlpha3*(180 - a3cap.theta(U[patchi], nHatp))
                )
            );

            vectorField nf(boundary[patchi].nf());

            // Reset nHatPatch to correspond to the contact angle

            scalarField a12(nHatp & nf);

            scalarField b1(cos(theta));

            scalarField b2(nHatp.size());

            forAll(b2, facei)
            {
                b2[facei] = cos(acos(a12[facei]) - theta[facei]);
            }

            scalarField det(1.0 - a12*a12);

            scalarField a((b1 - a12*b2)/det);
            scalarField b((b2 - a12*b1)/det);

            nHatp = a*nf + b*nHatp;

            nHatp /= (mag(nHatp) + deltaN_.value());
        }
    }
}


void Foam::threePhaseInterfaceProperties::calculateK()
{
    const volScalarField& alpha1 = mixture_.alpha1();

    const fvMesh& mesh = alpha1.mesh();
    const surfaceVectorField& Sf = mesh.Sf();

    // Cell gradient of alpha
    volVectorField gradAlpha(fvc::grad(alpha1));

    // Interpolated face-gradient of alpha
    surfaceVectorField gradAlphaf(fvc::interpolate(gradAlpha));

    // Face unit interface normal
    surfaceVectorField nHatfv(gradAlphaf/(mag(gradAlphaf) + deltaN_));

    correctContactAngle(nHatfv.boundaryFieldRef());

    // Face unit interface normal flux
    nHatf_ = nHatfv & Sf;

    // Simple expression for curvature
    K_ = -fvc::div(nHatf_);

    // Complex expression for curvature.
    // Correction is formally zero but numerically non-zero.
    // volVectorField nHat = gradAlpha/(mag(gradAlpha) + deltaN_);
    // nHat.boundaryFieldRef() = nHatfv.boundaryField();
    // K_ = -fvc::div(nHatf_) + (nHat & fvc::grad(nHatfv) & nHat);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::threePhaseInterfaceProperties::threePhaseInterfaceProperties
(
    const incompressibleThreePhaseMixture& mixture
)
:
    mixture_(mixture),
    cAlpha_
    (
        mixture.U().mesh().solverDict
        (
            mixture_.alpha1().name()
        ).get<scalar>("cAlpha")
    ),
    sigma12_("sigma12", dimMass/sqr(dimTime), mixture),
    sigma13_("sigma13", dimMass/sqr(dimTime), mixture),

    deltaN_
    (
        "deltaN",
        1e-8/cbrt(average(mixture.U().mesh().V()))
    ),

    nHatf_
    (
        IOobject
        (
            "nHatf",
            mixture.alpha1().time().timeName(),
            mixture.alpha1().mesh()
        ),
        mixture.alpha1().mesh(),
        dimensionedScalar(dimArea, Zero)
    ),

    K_
    (
        IOobject
        (
            "interfaceProperties:K",
            mixture.alpha1().time().timeName(),
            mixture.alpha1().mesh()
        ),
        mixture.alpha1().mesh(),
        dimensionedScalar(dimless/dimLength, Zero)
    )
{
    calculateK();
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::tmp<Foam::surfaceScalarField>
Foam::threePhaseInterfaceProperties::surfaceTensionForce() const
{
    return fvc::interpolate(sigmaK())*fvc::snGrad(mixture_.alpha1());
}


Foam::tmp<Foam::volScalarField>
Foam::threePhaseInterfaceProperties::nearInterface() const
{
    return max
    (
        pos0(mixture_.alpha1() - 0.01)*pos0(0.99 - mixture_.alpha1()),
        pos0(mixture_.alpha2() - 0.01)*pos0(0.99 - mixture_.alpha2())
    );
}


// ************************************************************************* //
