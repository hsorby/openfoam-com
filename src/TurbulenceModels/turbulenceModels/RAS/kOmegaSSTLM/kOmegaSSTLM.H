/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016 OpenFOAM Foundation
    Copyright (C) 2019 OpenCFD Ltd.
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

Class
    Foam::RASModels::kOmegaSSTLM

Group
    grpLESTurbulence

Description
    Langtry-Menter 4-equation transitional SST model
    based on the k-omega-SST RAS model.

    References:
    \verbatim
        Langtry, R. B., & Menter, F. R. (2009).
        Correlation-based transition modeling for unstructured parallelized
        computational fluid dynamics codes.
        AIAA journal, 47(12), 2894-2906.

        Menter, F. R., Langtry, R., & Volker, S. (2006).
        Transition modelling for general purpose CFD codes.
        Flow, turbulence and combustion, 77(1-4), 277-303.

        Langtry, R. B. (2006).
        A correlation-based transition model using local variables for
        unstructured parallelized CFD codes.
        Phd. Thesis, Universität Stuttgart.
    \endverbatim

    The model coefficients are
    \verbatim
        kOmegaSSTCoeffs
        {
            // Default SST coefficients
            alphaK1     0.85;
            alphaK2     1;
            alphaOmega1 0.5;
            alphaOmega2 0.856;
            beta1       0.075;
            beta2       0.0828;
            betaStar    0.09;
            gamma1      5/9;
            gamma2      0.44;
            a1          0.31;
            b1          1;
            c1          10;
            F3          no;

            // Default LM coefficients
            ca1         2;
            ca2         0.06;
            ce1         1;
            ce2         50;
            cThetat     0.03;
            sigmaThetat 2;

            lambdaErr   1e-6;
            maxLambdaIter 10;
        }
    \endverbatim

SourceFiles
    kOmegaSSTLM.C

\*---------------------------------------------------------------------------*/

#ifndef kOmegaSSTLM_H
#define kOmegaSSTLM_H

#include "RAS/kOmegaSST/kOmegaSST.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace RASModels
{

/*---------------------------------------------------------------------------*\
                     Class kOmegaSSTLM Declaration
\*---------------------------------------------------------------------------*/

template<class BasicTurbulenceModel>
class kOmegaSSTLM
:
    public kOmegaSST<BasicTurbulenceModel>
{
    // Private Member Functions

        //- No copy construct
        kOmegaSSTLM(const kOmegaSSTLM&) = delete;

        //- No copy assignment
        void operator=(const kOmegaSSTLM&) = delete;


protected:

    // Protected data

        // Model constants

            dimensionedScalar ca1_;
            dimensionedScalar ca2_;

            dimensionedScalar ce1_;
            dimensionedScalar ce2_;

            dimensionedScalar cThetat_;
            dimensionedScalar sigmaThetat_;

            //- Convergence criterion for the lambda/thetat loop
            scalar lambdaErr_;

            //- Maximum number of iterations to converge the lambda/thetat loop
            label maxLambdaIter_;

            //- Stabilization for division by the magnitude of the velocity
            const dimensionedScalar deltaU_;


        // Fields

            //- Transition onset momentum-thickness Reynolds number
            volScalarField ReThetat_;

            //- Intermittency
            volScalarField gammaInt_;

            //- Effective intermittency
            volScalarField::Internal gammaIntEff_;


    // Protected Member Functions

        //- Modified form of the k-omega SST F1 function
        virtual tmp<volScalarField> F1(const volScalarField& CDkOmega) const;

        //- Modified form of the k-omega SST k production rate
        virtual tmp<volScalarField::Internal> Pk
        (
            const volScalarField::Internal& G
        ) const;

        //- Modified form of the k-omega SST epsilon/k
        virtual tmp<volScalarField::Internal> epsilonByk
        (
            const volScalarField& F1,
            const volTensorField& gradU
        ) const;

        //- Freestream blending-function
        tmp<volScalarField::Internal> Fthetat
        (
            const volScalarField::Internal& Us,
            const volScalarField::Internal& Omega,
            const volScalarField::Internal& nu
        ) const;

        //- Empirical correlation for critical Reynolds number where the
        //  intermittency first starts to increase in the boundary layer
        tmp<volScalarField::Internal> ReThetac() const;

        //- Empirical correlation that controls the length of the
        //  transition region
        tmp<volScalarField::Internal> Flength
        (
            const volScalarField::Internal& nu
        ) const;

        //- Transition onset location control function
        tmp<volScalarField::Internal> Fonset
        (
            const volScalarField::Internal& Rev,
            const volScalarField::Internal& ReThetac,
            const volScalarField::Internal& RT
        ) const;

        //- Return the transition onset momentum-thickness Reynolds number
        // (based on freestream conditions)
        tmp<volScalarField::Internal> ReThetat0
        (
            const volScalarField::Internal& Us,
            const volScalarField::Internal& dUsds,
            const volScalarField::Internal& nu
        ) const;

        //- Solve the turbulence equations and correct the turbulence viscosity
        void correctReThetatGammaInt();


public:

    typedef typename BasicTurbulenceModel::alphaField alphaField;
    typedef typename BasicTurbulenceModel::rhoField rhoField;
    typedef typename BasicTurbulenceModel::transportModel transportModel;


    //- Runtime type information
    TypeName("kOmegaSSTLM");


    // Constructors

        //- Construct from components
        kOmegaSSTLM
        (
            const alphaField& alpha,
            const rhoField& rho,
            const volVectorField& U,
            const surfaceScalarField& alphaRhoPhi,
            const surfaceScalarField& phi,
            const transportModel& transport,
            const word& propertiesName = turbulenceModel::propertiesName,
            const word& type = typeName
        );


    //- Destructor
    virtual ~kOmegaSSTLM() = default;


    // Member Functions

        //- Re-read model coefficients if they have changed
        virtual bool read();

        //- Access function transition onset momentum-thickness Reynolds number
        const volScalarField& ReThetat() const
        {
            return ReThetat_;
        }

        //- Access function to intermittency
        const volScalarField& gammaInt() const
        {
            return gammaInt_;
        }

        //- Return the effective diffusivity for transition onset
        //  momentum-thickness Reynolds number
        tmp<volScalarField> DReThetatEff() const
        {
            return tmp<volScalarField>
            (
                new volScalarField
                (
                    "DReThetatEff",
                    sigmaThetat_*(this->nut_ + this->nu())
                )
            );
        }

        //- Return the effective diffusivity for intermittency
        tmp<volScalarField> DgammaIntEff() const
        {
            return tmp<volScalarField>
            (
                new volScalarField
                (
                    "DgammaIntEff",
                    this->nut_ + this->nu()
                )
            );
        }

        //- Solve the turbulence equations and correct the turbulence viscosity
        virtual void correct();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "RAS/kOmegaSSTLM/kOmegaSSTLM.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
