/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2015-2019 OpenFOAM Foundation
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

#include "derivedFvPatchFields/alphatPhaseChangeJayatillekeWallFunction/alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField.H"
#include "fields/fvPatchFields/fvPatchField/fvPatchFieldMapper.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"

#include "phaseSystem/phaseSystem.H"
#include "compressibleTurbulenceModel.H"
#include "ThermalDiffusivity/ThermalDiffusivity.H"
#include "TurbulenceModels/phaseCompressible/PhaseCompressibleTurbulenceModel/PhaseCompressibleTurbulenceModelPascal.H"
#include "fvMesh/fvPatches/derived/wall/wallFvPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace compressible
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

scalar alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::tolerance_
    = 0.01;
label alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::maxIters_
    = 10;

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::checkType()
{
    if (!isA<wallFvPatch>(patch()))
    {
        FatalErrorInFunction
            << "Patch type for patch " << patch().name() << " must be wall\n"
            << "Current patch type is " << patch().type() << nl
            << exit(FatalError);
    }
}


tmp<scalarField>
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::Psmooth
(
    const scalarField& Prat
) const
{
    return 9.24*(pow(Prat, 0.75) - 1)*(1 + 0.28*exp(-0.007*Prat));
}


tmp<scalarField>
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::yPlusTherm
(
    const scalarField& P,
    const scalarField& Prat
) const
{
    auto typsf = tmp<scalarField>::New(this->size());
    auto& ypsf = typsf.ref();

    forAll(ypsf, facei)
    {
        scalar ypt = 11.0;

        for (int i = 0; i < maxIters_; ++i)
        {
            const scalar f = ypt - (log(E_*ypt)/kappa_ + P[facei])/Prat[facei];
            const scalar df = 1.0 - 1.0/(ypt*kappa_*Prat[facei]);
            const scalar yptNew = ypt - f/df;

            if (yptNew < VSMALL)
            {
                ypsf[facei] = 0;
            }
            else if (mag(yptNew - ypt) < tolerance_)
            {
                ypsf[facei] = yptNew;
            }
            else
            {
                ypt = yptNew;
            }
        }

        ypsf[facei] = ypt;
    }

    return typsf;
}


tmp<scalarField>
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::calcAlphat
(
    const scalarField& prevAlphat
) const
{
    // Lookup the fluid model
    const phaseSystem& fluid =
        db().lookupObject<phaseSystem>("phaseProperties");

    const phaseModel& phase = fluid.phases()[internalField().group()];

    const label patchi = patch().index();

    // Retrieve turbulence properties from model
    const auto& turbModel =
        db().lookupObject<phaseCompressibleTurbulenceModel>
        (
            IOobject::groupName(turbulenceModel::propertiesName, phase.name())
        );

    const scalar Cmu25 = pow025(Cmu_);

    const scalarField& y = turbModel.y()[patchi];

    const tmp<scalarField> tmuw = turbModel.mu(patchi);
    const scalarField& muw = tmuw();

    const tmp<scalarField> talphaw = phase.thermo().alpha(patchi);
    const scalarField& alphaw = talphaw();

    const tmp<volScalarField> tk = turbModel.k();
    const volScalarField& k = tk();
    const fvPatchScalarField& kw = k.boundaryField()[patchi];

    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchi];
    const scalarField magUp(mag(Uw.patchInternalField() - Uw));
    const scalarField magGradUw(mag(Uw.snGrad()));

    const fvPatchScalarField& rhow = turbModel.rho().boundaryField()[patchi];
    const fvPatchScalarField& hew =
        phase.thermo().he().boundaryField()[patchi];

    // Heat flux [W/m2] - lagging alphatw
    const scalarField qDot
    (
        (prevAlphat + alphaw)*hew.snGrad()
    );

    scalarField uTau(Cmu25*sqrt(kw));

    scalarField yPlus(uTau*y/(muw/rhow));

    const scalarField Pr(muw/alphaw);

    // Molecular-to-turbulent Prandtl number ratio
    const scalarField Prat(Pr/Prt_);

    // Thermal sublayer thickness
    const scalarField P(this->Psmooth(Prat));

    tmp<scalarField> tyPlusTherm = this->yPlusTherm(P, Prat);
    const scalarField& yPlusTherm = tyPlusTherm.cref();

    auto talphatConv = tmp<scalarField>::New(this->size());
    auto& alphatConv = talphatConv.ref();

    // Populate boundary values
    forAll(alphatConv, facei)
    {
        // Evaluate new effective thermal diffusivity
        scalar alphaEff = 0.0;
        if (yPlus[facei] < yPlusTherm[facei])
        {
            const scalar A = qDot[facei]*rhow[facei]*uTau[facei]*y[facei];
            const scalar B = qDot[facei]*Pr[facei]*yPlus[facei];
            const scalar C =
                Pr[facei]*0.5*rhow[facei]*uTau[facei]*sqr(magUp[facei]);
            alphaEff = A/(B + C + VSMALL);
        }
        else
        {
            const scalar A = qDot[facei]*rhow[facei]*uTau[facei]*y[facei];
            const scalar B =
                qDot[facei]*Prt_*(1.0/kappa_*log(E_*yPlus[facei]) + P[facei]);
            const scalar magUc =
                uTau[facei]/kappa_*log(E_*yPlusTherm[facei]) - mag(Uw[facei]);
            const scalar C =
                0.5*rhow[facei]*uTau[facei]
               *(Prt_*sqr(magUp[facei]) + (Pr[facei] - Prt_)*sqr(magUc));
            alphaEff = A/(B + C + VSMALL);
        }

        // Update convective heat transfer turbulent thermal diffusivity
        alphatConv[facei] = max(0.0, alphaEff - alphaw[facei]);
    }

    return talphatConv;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    alphatPhaseChangeWallFunctionFvPatchScalarField(p, iF),
    Prt_(0.85),
    Cmu_(0.09),
    kappa_(0.41),
    E_(9.8)
{
    checkType();
}


alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    alphatPhaseChangeWallFunctionFvPatchScalarField(p, iF, dict),
    Prt_(dict.getOrDefault<scalar>("Prt", 0.85)),
    Cmu_(dict.getOrDefault<scalar>("Cmu", 0.09)),
    kappa_(dict.getOrDefault<scalar>("kappa", 0.41)),
    E_(dict.getOrDefault<scalar>("E", 9.8))
{}


alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField
(
    const alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    alphatPhaseChangeWallFunctionFvPatchScalarField(ptf, p, iF, mapper),
    Prt_(ptf.Prt_),
    Cmu_(ptf.Cmu_),
    kappa_(ptf.kappa_),
    E_(ptf.E_)
{}


alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField
(
    const alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField& awfpsf
)
:
    alphatPhaseChangeWallFunctionFvPatchScalarField(awfpsf),
    Prt_(awfpsf.Prt_),
    Cmu_(awfpsf.Cmu_),
    kappa_(awfpsf.kappa_),
    E_(awfpsf.E_)
{}


alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::
alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField
(
    const alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField& awfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    alphatPhaseChangeWallFunctionFvPatchScalarField(awfpsf, iF),
    Prt_(awfpsf.Prt_),
    Cmu_(awfpsf.Cmu_),
    kappa_(awfpsf.kappa_),
    E_(awfpsf.E_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    operator==(calcAlphat(*this));

    fixedValueFvPatchScalarField::updateCoeffs();
}


void alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField::write
(
    Ostream& os
) const
{
    fvPatchField<scalar>::write(os);
    os.writeEntry("Prt", Prt_);
    os.writeEntry("Cmu", Cmu_);
    os.writeEntry("kappa", kappa_);
    os.writeEntry("E", E_);
    dmdt_.writeEntry("dmdt", os);
    fvPatchField<scalar>::writeValueEntry(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    alphatPhaseChangeJayatillekeWallFunctionFvPatchScalarField
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace compressible
} // End namespace Foam

// ************************************************************************* //
