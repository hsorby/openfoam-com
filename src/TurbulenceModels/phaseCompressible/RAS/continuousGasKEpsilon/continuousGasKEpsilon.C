/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2013-2017 OpenFOAM Foundation
    Copyright (C) 2019-2023 OpenCFD Ltd.
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

#include "TurbulenceModels/phaseCompressible/RAS/continuousGasKEpsilon/continuousGasKEpsilon.H"
#include "cfdTools/general/fvOptions/fvOptions.H"
#include "twoPhaseSystem.H"
#include "interfacialModels/virtualMassModels/virtualMassModel/virtualMassModel.H"
#include "interfacialModels/dragModels/dragModel/dragModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace RASModels
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class BasicTurbulenceModel>
continuousGasKEpsilon<BasicTurbulenceModel>::continuousGasKEpsilon
(
    const alphaField& alpha,
    const rhoField& rho,
    const volVectorField& U,
    const surfaceScalarField& alphaRhoPhi,
    const surfaceScalarField& phi,
    const transportModel& transport,
    const word& propertiesName,
    const word& type
)
:
    kEpsilon<BasicTurbulenceModel>
    (
        alpha,
        rho,
        U,
        alphaRhoPhi,
        phi,
        transport,
        propertiesName,
        type
    ),

    liquidTurbulencePtr_(nullptr),

    nutEff_
    (
        IOobject
        (
            IOobject::groupName("nutEff", alphaRhoPhi.group()),
            this->runTime_.timeName(),
            this->mesh_,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        this->nut_
    ),

    alphaInversion_
    (
        dimensioned<scalar>::getOrAddToDict
        (
            "alphaInversion",
            this->coeffDict_,
            0.7
        )
    )
{
    if (type == typeName)
    {
        this->printCoeffs(type);
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class BasicTurbulenceModel>
bool continuousGasKEpsilon<BasicTurbulenceModel>::read()
{
    if (kEpsilon<BasicTurbulenceModel>::read())
    {
        alphaInversion_.readIfPresent(this->coeffDict());

        return true;
    }

    return false;
}


template<class BasicTurbulenceModel>
void continuousGasKEpsilon<BasicTurbulenceModel>::correctNut()
{
    kEpsilon<BasicTurbulenceModel>::correctNut();

    const turbulenceModel& liquidTurbulence = this->liquidTurbulence();
    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());
    const transportModel& liquid = fluid.otherPhase(gas);

     const virtualMassModel& virtualMass =
        fluid.lookupSubModel<virtualMassModel>(gas, liquid);

    volScalarField thetal(liquidTurbulence.k()/liquidTurbulence.epsilon());
    volScalarField rhodv(gas.rho() + virtualMass.Cvm()*liquid.rho());
    volScalarField thetag((rhodv/(18*liquid.rho()*liquid.nu()))*sqr(gas.d()));
    volScalarField expThetar
    (
        min
        (
            exp(min(thetal/thetag, scalar(50))),
            scalar(1)
        )
    );
    volScalarField omega((1 - expThetar)/(1 + expThetar));

    nutEff_ = omega*liquidTurbulence.nut();
    fv::options::New(this->mesh_).correct(nutEff_);
}


template<class BasicTurbulenceModel>
const turbulenceModel&
continuousGasKEpsilon<BasicTurbulenceModel>::liquidTurbulence() const
{
    if (!liquidTurbulencePtr_)
    {
        const volVectorField& U = this->U_;

        const transportModel& gas = this->transport();
        const twoPhaseSystem& fluid =
            refCast<const twoPhaseSystem>(gas.fluid());
        const transportModel& liquid = fluid.otherPhase(gas);

        liquidTurbulencePtr_ =
           &U.db().lookupObject<turbulenceModel>
            (
                IOobject::groupName
                (
                    turbulenceModel::propertiesName,
                    liquid.name()
                )
            );
    }

    return *liquidTurbulencePtr_;
}


template<class BasicTurbulenceModel>
tmp<Foam::volScalarField>
continuousGasKEpsilon<BasicTurbulenceModel>::nuEff() const
{
    volScalarField blend
    (
        max
        (
            min
            (
                (this->alpha_ - scalar(0.5))/(alphaInversion_ - 0.5),
                scalar(1)
            ),
            scalar(0)
        )
    );

    return tmp<volScalarField>
    (
        new volScalarField
        (
            IOobject::groupName("nuEff", this->alphaRhoPhi_.group()),
            blend*this->nut_
          + (1.0 - blend)*rhoEff()*nutEff_/this->transport().rho()
          + this->nu()
        )
    );
}


template<class BasicTurbulenceModel>
tmp<Foam::volScalarField>
continuousGasKEpsilon<BasicTurbulenceModel>::rhoEff() const
{
    const transportModel& gas = this->transport();
    const twoPhaseSystem& fluid = refCast<const twoPhaseSystem>(gas.fluid());
    const transportModel& liquid = fluid.otherPhase(gas);

    const virtualMassModel& virtualMass =
        fluid.lookupSubModel<virtualMassModel>(gas, liquid);

    return tmp<volScalarField>
    (
        new volScalarField
        (
            IOobject::groupName("rhoEff", this->alphaRhoPhi_.group()),
            gas.rho() + (virtualMass.Cvm() + 3.0/20.0)*liquid.rho()
        )
    );
}


template<class BasicTurbulenceModel>
tmp<volScalarField>
continuousGasKEpsilon<BasicTurbulenceModel>::phaseTransferCoeff() const
{
    const volVectorField& U = this->U_;
    const alphaField& alpha = this->alpha_;
    const rhoField& rho = this->rho_;

    const turbulenceModel& liquidTurbulence = this->liquidTurbulence();

    return
    (
        max(alphaInversion_ - alpha, scalar(0))
       *rho
       *min
        (
            liquidTurbulence.epsilon()/liquidTurbulence.k(),
            1.0/U.time().deltaT()
        )
    );
}


template<class BasicTurbulenceModel>
tmp<fvScalarMatrix>
continuousGasKEpsilon<BasicTurbulenceModel>::kSource() const
{
    const turbulenceModel& liquidTurbulence = this->liquidTurbulence();
    const volScalarField phaseTransferCoeff(this->phaseTransferCoeff());

    return
        phaseTransferCoeff*liquidTurbulence.k()
      - fvm::Sp(phaseTransferCoeff, this->k_);
}


template<class BasicTurbulenceModel>
tmp<fvScalarMatrix>
continuousGasKEpsilon<BasicTurbulenceModel>::epsilonSource() const
{
    const turbulenceModel& liquidTurbulence = this->liquidTurbulence();
    const volScalarField phaseTransferCoeff(this->phaseTransferCoeff());

    return
        phaseTransferCoeff*liquidTurbulence.epsilon()
      - fvm::Sp(phaseTransferCoeff, this->epsilon_);
}


template<class BasicTurbulenceModel>
tmp<volSymmTensorField>
continuousGasKEpsilon<BasicTurbulenceModel>::R() const
{
    tmp<volScalarField> tk(this->k());

    return tmp<volSymmTensorField>
    (
        new volSymmTensorField
        (
            IOobject
            (
                IOobject::groupName("R", this->alphaRhoPhi_.group()),
                this->runTime_.timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            ((2.0/3.0)*I)*tk() - (nutEff_)*devTwoSymm(fvc::grad(this->U_)),
            tk().boundaryField().types()
        )
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace Foam

// ************************************************************************* //
