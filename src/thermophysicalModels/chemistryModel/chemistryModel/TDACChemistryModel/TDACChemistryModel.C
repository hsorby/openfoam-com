/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016-2021 OpenFOAM Foundation
    Copyright (C) 2016-2021 OpenCFD Ltd.
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

#include "chemistryModel/TDACChemistryModel/TDACChemistryModel.H"
#include "fields/Fields/UniformField/UniformField.H"
#include "finiteVolume/ddtSchemes/localEulerDdtScheme/localEulerDdtScheme.H"
#include "global/clockTime/clockTime.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class ReactionThermo, class ThermoType>
Foam::TDACChemistryModel<ReactionThermo, ThermoType>::TDACChemistryModel
(
    ReactionThermo& thermo
)
:
    StandardChemistryModel<ReactionThermo, ThermoType>(thermo),
    variableTimeStep_
    (
        this->mesh().time().controlDict().getOrDefault
        (
            "adjustTimeStep",
            false
        )
     || fv::localEulerDdt::enabled(this->mesh())
    ),
    timeSteps_(0),
    NsDAC_(this->nSpecie_),
    completeC_(this->nSpecie_, 0),
    reactionsDisabled_(this->reactions_.size(), false),
    specieComp_(this->nSpecie_),
    completeToSimplifiedIndex_(this->nSpecie_, -1),
    simplifiedToCompleteIndex_(this->nSpecie_),
    tabulationResults_
    (
        IOobject
        (
            thermo.phasePropertyName("TabulationResults"),
            this->time().timeName(),
            this->mesh(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        this->mesh(),
        dimensionedScalar(dimless, Zero)
    )
{
    basicSpecieMixture& composition = this->thermo().composition();

    // Store the species composition according to the species index
    speciesTable speciesTab = composition.species();

    autoPtr<speciesCompositionTable> specCompPtr
    (
        dynamicCast<const reactingMixture<ThermoType>>(this->thermo())
       .specieComposition()
    );

    forAll(specieComp_, i)
    {
        specieComp_[i] = (specCompPtr.ref())[this->Y()[i].member()];
    }

    mechRed_ = chemistryReductionMethod<ReactionThermo, ThermoType>::New
    (
        *this,
        *this
    );

    // When the mechanism reduction method is used, the 'active' flag for every
    // species should be initialized (by default 'active' is true)
    if (mechRed_->active())
    {
        forAll(this->Y(), i)
        {
            IOobject header
            (
                this->Y()[i].name(),
                this->mesh().time().timeName(),
                this->mesh(),
                IOobject::NO_READ
            );

            // Check if the species file is provided, if not set inactive
            // and NO_WRITE
            if (!header.typeHeaderOk<volScalarField>(true))
            {
                composition.setInactive(i);
                this->Y()[i].writeOpt(IOobject::NO_WRITE);
            }
        }
    }

    tabulation_ = chemistryTabulationMethod<ReactionThermo, ThermoType>::New
    (
        *this,
        *this
    );

    if (mechRed_->log())
    {
        cpuReduceFile_ = logFile("cpu_reduce.out");
        nActiveSpeciesFile_ = logFile("nActiveSpecies.out");
    }

    if (tabulation_->log())
    {
        cpuAddFile_ = logFile("cpu_add.out");
        cpuGrowFile_ = logFile("cpu_grow.out");
        cpuRetrieveFile_ = logFile("cpu_retrieve.out");
    }

    if (mechRed_->log() || tabulation_->log())
    {
        cpuSolveFile_ = logFile("cpu_solve.out");
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class ReactionThermo, class ThermoType>
Foam::TDACChemistryModel<ReactionThermo, ThermoType>::~TDACChemistryModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::omega
(
    const scalarField& c, // Contains all species even when mechRed is active
    const scalar T,
    const scalar p,
    scalarField& dcdt
) const
{
    const bool reduced = mechRed_->active();

    scalar pf, cf, pr, cr;
    label lRef, rRef;

    dcdt = Zero;

    forAll(this->reactions_, i)
    {
        if (!reactionsDisabled_[i])
        {
            const Reaction<ThermoType>& R = this->reactions_[i];

            scalar omegai = omega
            (
                R, c, T, p, pf, cf, lRef, pr, cr, rRef
            );

            forAll(R.lhs(), s)
            {
                label si = R.lhs()[s].index;
                if (reduced)
                {
                    si = completeToSimplifiedIndex_[si];
                }

                const scalar sl = R.lhs()[s].stoichCoeff;
                dcdt[si] -= sl*omegai;
            }
            forAll(R.rhs(), s)
            {
                label si = R.rhs()[s].index;
                if (reduced)
                {
                    si = completeToSimplifiedIndex_[si];
                }

                const scalar sr = R.rhs()[s].stoichCoeff;
                dcdt[si] += sr*omegai;
            }
        }
    }
}


template<class ReactionThermo, class ThermoType>
Foam::scalar Foam::TDACChemistryModel<ReactionThermo, ThermoType>::omega
(
    const Reaction<ThermoType>& R,
    const scalarField& c, // Contains all species even when mechRed is active
    const scalar T,
    const scalar p,
    scalar& pf,
    scalar& cf,
    label& lRef,
    scalar& pr,
    scalar& cr,
    label& rRef
) const
{
    const scalar kf = R.kf(p, T, c);
    const scalar kr = R.kr(kf, p, T, c);

    const label Nl = R.lhs().size();
    const label Nr = R.rhs().size();

    label slRef = 0;
    lRef = R.lhs()[slRef].index;

    pf = kf;
    for (label s=1; s<Nl; s++)
    {
        const label si = R.lhs()[s].index;

        if (c[si] < c[lRef])
        {
            const scalar exp = R.lhs()[slRef].exponent;
            pf *= pow(max(c[lRef], 0.0), exp);
            lRef = si;
            slRef = s;
        }
        else
        {
            const scalar exp = R.lhs()[s].exponent;
            pf *= pow(max(c[si], 0.0), exp);
        }
    }
    cf = max(c[lRef], 0.0);

    {
        const scalar exp = R.lhs()[slRef].exponent;
        if (exp < 1)
        {
            if (cf > SMALL)
            {
                pf *= pow(cf, exp - 1);
            }
            else
            {
                pf = 0;
            }
        }
        else
        {
            pf *= pow(cf, exp - 1);
        }
    }

    label srRef = 0;
    rRef = R.rhs()[srRef].index;

    // Find the matrix element and element position for the rhs
    pr = kr;
    for (label s=1; s<Nr; s++)
    {
        const label si = R.rhs()[s].index;
        if (c[si] < c[rRef])
        {
            const scalar exp = R.rhs()[srRef].exponent;
            pr *= pow(max(c[rRef], 0.0), exp);
            rRef = si;
            srRef = s;
        }
        else
        {
            const scalar exp = R.rhs()[s].exponent;
            pr *= pow(max(c[si], 0.0), exp);
        }
    }
    cr = max(c[rRef], 0.0);

    {
        const scalar exp = R.rhs()[srRef].exponent;
        if (exp < 1)
        {
            if (cr > SMALL)
            {
                pr *= pow(cr, exp - 1);
            }
            else
            {
                pr = 0;
            }
        }
        else
        {
            pr *= pow(cr, exp - 1);
        }
    }

    return pf*cf - pr*cr;
}


template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::derivatives
(
    const scalar time,
    const scalarField& c,
    scalarField& dcdt
) const
{
    const bool reduced = mechRed_->active();

    const scalar T = c[this->nSpecie_];
    const scalar p = c[this->nSpecie_ + 1];

    if (reduced)
    {
        // When using DAC, the ODE solver submit a reduced set of species the
        // complete set is used and only the species in the simplified mechanism
        // are updated
        this->c_ = completeC_;

        // Update the concentration of the species in the simplified mechanism
        // the other species remain the same and are used only for third-body
        // efficiencies
        for (label i=0; i<NsDAC_; i++)
        {
            this->c_[simplifiedToCompleteIndex_[i]] = max(c[i], 0.0);
        }
    }
    else
    {
        for (label i=0; i<this->nSpecie(); i++)
        {
            this->c_[i] = max(c[i], 0.0);
        }
    }

    omega(this->c_, T, p, dcdt);

    // Constant pressure
    // dT/dt = ...
    scalar rho = 0;
    for (label i=0; i<this->c_.size(); i++)
    {
        const scalar W = this->specieThermo_[i].W();
        rho += W*this->c_[i];
    }

    scalar cp = 0;
    for (label i=0; i<this->c_.size(); i++)
    {
        // cp function returns [J/(kmol K)]
        cp += this->c_[i]*this->specieThermo_[i].cp(p, T);
    }
    cp /= rho;

    // When mechanism reduction is active
    // dT is computed on the reduced set since dcdt is null
    // for species not involved in the simplified mechanism
    scalar dT = 0;
    for (label i=0; i<this->nSpecie_; i++)
    {
        label si;
        if (reduced)
        {
            si = simplifiedToCompleteIndex_[i];
        }
        else
        {
            si = i;
        }

        // ha function returns [J/kmol]
        const scalar hi = this->specieThermo_[si].ha(p, T);
        dT += hi*dcdt[i];
    }
    dT /= rho*cp;

    dcdt[this->nSpecie_] = -dT;

    // dp/dt = ...
    dcdt[this->nSpecie_ + 1] = 0;
}


template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::jacobian
(
    const scalar t,
    const scalarField& c,
    scalarSquareMatrix& dfdc
) const
{
    const bool reduced = mechRed_->active();

    // If the mechanism reduction is active, the computed Jacobian
    // is compact (size of the reduced set of species)
    // but according to the information of the complete set
    // (i.e. for the third-body efficiencies)

    const label nSpecie = this->nSpecie_;

    const scalar T = c[nSpecie];
    const scalar p = c[nSpecie + 1];

    if (reduced)
    {
        this->c_ = completeC_;
        for (label i=0; i<NsDAC_; ++i)
        {
            this->c_[simplifiedToCompleteIndex_[i]] = max(c[i], 0.0);
        }
    }
    else
    {
        forAll(this->c_, i)
        {
            this->c_[i] = max(c[i], 0.0);
        }
    }

    dfdc = Zero;

    forAll(this->reactions_, ri)
    {
        if (!reactionsDisabled_[ri])
        {
            const Reaction<ThermoType>& R = this->reactions_[ri];

            const scalar kf0 = R.kf(p, T, this->c_);
            const scalar kr0 = R.kr(kf0, p, T, this->c_);

            forAll(R.lhs(), j)
            {
                label sj = R.lhs()[j].index;
                if (reduced)
                {
                    sj = completeToSimplifiedIndex_[sj];
                }
                scalar kf = kf0;
                forAll(R.lhs(), i)
                {
                    const label si = R.lhs()[i].index;
                    const scalar el = R.lhs()[i].exponent;
                    if (i == j)
                    {
                        if (el < 1)
                        {
                            if (this->c_[si] > SMALL)
                            {
                                kf *= el*pow(this->c_[si], el - 1);
                            }
                            else
                            {
                                kf = 0;
                            }
                        }
                        else
                        {
                            kf *= el*pow(this->c_[si], el - 1);
                        }
                    }
                    else
                    {
                        kf *= pow(this->c_[si], el);
                    }
                }

                forAll(R.lhs(), i)
                {
                    label si = R.lhs()[i].index;
                    if (reduced)
                    {
                        si = completeToSimplifiedIndex_[si];
                    }
                    const scalar sl = R.lhs()[i].stoichCoeff;
                    dfdc(si, sj) -= sl*kf;
                }
                forAll(R.rhs(), i)
                {
                    label si = R.rhs()[i].index;
                    if (reduced)
                    {
                        si = completeToSimplifiedIndex_[si];
                    }
                    const scalar sr = R.rhs()[i].stoichCoeff;
                    dfdc(si, sj) += sr*kf;
                }
            }

            forAll(R.rhs(), j)
            {
                label sj = R.rhs()[j].index;
                if (reduced)
                {
                    sj = completeToSimplifiedIndex_[sj];
                }
                scalar kr = kr0;
                forAll(R.rhs(), i)
                {
                    const label si = R.rhs()[i].index;
                    const scalar er = R.rhs()[i].exponent;
                    if (i == j)
                    {
                        if (er < 1)
                        {
                            if (this->c_[si] > SMALL)
                            {
                                kr *= er*pow(this->c_[si], er - 1);
                            }
                            else
                            {
                                kr = 0;
                            }
                        }
                        else
                        {
                            kr *= er*pow(this->c_[si], er - 1);
                        }
                    }
                    else
                    {
                        kr *= pow(this->c_[si], er);
                    }
                }

                forAll(R.lhs(), i)
                {
                    label si = R.lhs()[i].index;
                    if (reduced)
                    {
                        si = completeToSimplifiedIndex_[si];
                    }
                    const scalar sl = R.lhs()[i].stoichCoeff;
                    dfdc(si, sj) += sl*kr;
                }
                forAll(R.rhs(), i)
                {
                    label si = R.rhs()[i].index;
                    if (reduced)
                    {
                        si = completeToSimplifiedIndex_[si];
                    }
                    const scalar sr = R.rhs()[i].stoichCoeff;
                    dfdc(si, sj) -= sr*kr;
                }
            }
        }
    }

    // Calculate the dcdT elements numerically
    const scalar delta = 1e-3;

    omega(this->c_, T + delta, p, this->dcdt_);
    for (label i=0; i<nSpecie; ++i)
    {
        dfdc(i, nSpecie) = this->dcdt_[i];
    }

    omega(this->c_, T - delta, p, this->dcdt_);
    for (label i=0; i<nSpecie; ++i)
    {
        dfdc(i, nSpecie) = 0.5*(dfdc(i, nSpecie) - this->dcdt_[i])/delta;
    }

    dfdc(nSpecie, nSpecie) = 0;
    dfdc(nSpecie + 1, nSpecie) = 0;
}


template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::jacobian
(
    const scalar t,
    const scalarField& c,
    scalarField& dcdt,
    scalarSquareMatrix& dfdc
) const
{
    jacobian(t, c, dfdc);

    const scalar T = c[this->nSpecie_];
    const scalar p = c[this->nSpecie_ + 1];

    // Note: Uses the c_ field initialized by the call to jacobian above
    omega(this->c_, T, p, dcdt);
}


template<class ReactionThermo, class ThermoType>
template<class DeltaTType>
Foam::scalar Foam::TDACChemistryModel<ReactionThermo, ThermoType>::solve
(
    const DeltaTType& deltaT
)
{
    // Increment counter of time-step
    timeSteps_++;

    const bool reduced = mechRed_->active();

    label nAdditionalEqn = (tabulation_->variableTimeStep() ? 1 : 0);

    basicSpecieMixture& composition = this->thermo().composition();

    // CPU time analysis
    const clockTime clockTime_= clockTime();
    clockTime_.timeIncrement();
    scalar reduceMechCpuTime_ = 0;
    scalar addNewLeafCpuTime_ = 0;
    scalar growCpuTime_ = 0;
    scalar solveChemistryCpuTime_ = 0;
    scalar searchISATCpuTime_ = 0;

    this->resetTabulationResults();

    // Average number of active species
    scalar nActiveSpecies = 0;
    scalar nAvg = 0;

    BasicChemistryModel<ReactionThermo>::correct();

    scalar deltaTMin = GREAT;

    if (!this->chemistry_)
    {
        return deltaTMin;
    }

    const volScalarField rho
    (
        IOobject
        (
            "rho",
            this->time().timeName(),
            this->mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            IOobject::NO_REGISTER
        ),
        this->thermo().rho()
    );

    const scalarField& T = this->thermo().T();
    const scalarField& p = this->thermo().p();

    scalarField c(this->nSpecie_);
    scalarField c0(this->nSpecie_);

    // Composition vector (Yi, T, p)
    scalarField phiq(this->nEqns() + nAdditionalEqn);

    scalarField Rphiq(this->nEqns() + nAdditionalEqn);

    forAll(rho, celli)
    {
        const scalar rhoi = rho[celli];
        scalar pi = p[celli];
        scalar Ti = T[celli];

        for (label i=0; i<this->nSpecie_; i++)
        {
            const volScalarField& Yi = this->Y_[i];
            c[i] = rhoi*Yi[celli]/this->specieThermo_[i].W();
            c0[i] = c[i];
            phiq[i] = Yi[celli];
        }
        phiq[this->nSpecie()] = Ti;
        phiq[this->nSpecie() + 1] = pi;
        if (tabulation_->variableTimeStep())
        {
            phiq[this->nSpecie() + 2] = deltaT[celli];
        }


        // Initialise time progress
        scalar timeLeft = deltaT[celli];

        // Not sure if this is necessary
        Rphiq = Zero;

        clockTime_.timeIncrement();

        // When tabulation is active (short-circuit evaluation for retrieve)
        // It first tries to retrieve the solution of the system with the
        // information stored through the tabulation method
        if (tabulation_->active() && tabulation_->retrieve(phiq, Rphiq))
        {
            // Retrieved solution stored in Rphiq
            for (label i=0; i<this->nSpecie(); ++i)
            {
                c[i] = rhoi*Rphiq[i]/this->specieThermo_[i].W();
            }

            searchISATCpuTime_ += clockTime_.timeIncrement();
        }
        // This position is reached when tabulation is not used OR
        // if the solution is not retrieved.
        // In the latter case, it adds the information to the tabulation
        // (it will either expand the current data or add a new stored point).
        else
        {
            // Store total time waiting to attribute to add or grow
            scalar timeTmp = clockTime_.timeIncrement();

            if (reduced)
            {
                // Reduce mechanism change the number of species (only active)
                mechRed_->reduceMechanism(c, Ti, pi);
                nActiveSpecies += mechRed_->NsSimp();
                ++nAvg;
                scalar timeIncr = clockTime_.timeIncrement();
                reduceMechCpuTime_ += timeIncr;
                timeTmp += timeIncr;
            }

            // Calculate the chemical source terms
            while (timeLeft > SMALL)
            {
                scalar dt = timeLeft;
                if (reduced)
                {
                    // completeC_ used in the overridden ODE methods
                    // to update only the active species
                    completeC_ = c;

                    // Solve the reduced set of ODE
                    this->solve
                    (
                        simplifiedC_, Ti, pi, dt, this->deltaTChem_[celli]
                    );

                    for (label i=0; i<NsDAC_; ++i)
                    {
                        c[simplifiedToCompleteIndex_[i]] = simplifiedC_[i];
                    }
                }
                else
                {
                    this->solve(c, Ti, pi, dt, this->deltaTChem_[celli]);
                }
                timeLeft -= dt;
            }

            {
                scalar timeIncr = clockTime_.timeIncrement();
                solveChemistryCpuTime_ += timeIncr;
                timeTmp += timeIncr;
            }

            // If tabulation is used, we add the information computed here to
            // the stored points (either expand or add)
            if (tabulation_->active())
            {
                forAll(c, i)
                {
                    Rphiq[i] = c[i]/rhoi*this->specieThermo_[i].W();
                }
                if (tabulation_->variableTimeStep())
                {
                    Rphiq[Rphiq.size()-3] = Ti;
                    Rphiq[Rphiq.size()-2] = pi;
                    Rphiq[Rphiq.size()-1] = deltaT[celli];
                }
                else
                {
                    Rphiq[Rphiq.size()-2] = Ti;
                    Rphiq[Rphiq.size()-1] = pi;
                }
                label growOrAdd =
                    tabulation_->add(phiq, Rphiq, rhoi, deltaT[celli]);

                if (growOrAdd)
                {
                    this->setTabulationResultsAdd(celli);
                    addNewLeafCpuTime_ += clockTime_.timeIncrement() + timeTmp;
                }
                else
                {
                    this->setTabulationResultsGrow(celli);
                    growCpuTime_ += clockTime_.timeIncrement() + timeTmp;
                }
            }

            // When operations are done and if mechanism reduction is active,
            // the number of species (which also affects nEqns) is set back
            // to the total number of species (stored in the mechRed object)
            if (reduced)
            {
                this->nSpecie_ = mechRed_->nSpecie();
            }
            deltaTMin = min(this->deltaTChem_[celli], deltaTMin);

            this->deltaTChem_[celli] =
                min(this->deltaTChem_[celli], this->deltaTChemMax_);
        }

        // Set the RR vector (used in the solver)
        for (label i=0; i<this->nSpecie_; ++i)
        {
            this->RR_[i][celli] =
                (c[i] - c0[i])*this->specieThermo_[i].W()/deltaT[celli];
        }
    }

    if (mechRed_->log() || tabulation_->log())
    {
        cpuSolveFile_()
            << this->time().timeOutputValue()
            << "    " << solveChemistryCpuTime_ << endl;
    }

    if (mechRed_->log())
    {
        cpuReduceFile_()
            << this->time().timeOutputValue()
            << "    " << reduceMechCpuTime_ << endl;
    }

    if (tabulation_->active())
    {
        // Every time-step, look if the tabulation should be updated
        tabulation_->update();

        // Write the performance of the tabulation
        tabulation_->writePerformance();

        if (tabulation_->log())
        {
            cpuRetrieveFile_()
                << this->time().timeOutputValue()
                << "    " << searchISATCpuTime_ << endl;

            cpuGrowFile_()
                << this->time().timeOutputValue()
                << "    " << growCpuTime_ << endl;

            cpuAddFile_()
                << this->time().timeOutputValue()
                << "    " << addNewLeafCpuTime_ << endl;
        }
    }

    if (reduced && nAvg && mechRed_->log())
    {
        // Write average number of species
        nActiveSpeciesFile_()
            << this->time().timeOutputValue()
            << "    " << nActiveSpecies/nAvg << endl;
    }

    if (reduced && Pstream::parRun())
    {
        List<bool> active(composition.active());
        Pstream::listCombineReduce(active, orEqOp<bool>());

        forAll(active, i)
        {
            if (active[i])
            {
                composition.setActive(i);
            }
        }
    }

    forAll(this->Y(), i)
    {
        if (composition.active(i))
        {
            this->Y()[i].writeOpt(IOobject::AUTO_WRITE);
        }
    }

    return deltaTMin;
}


template<class ReactionThermo, class ThermoType>
Foam::scalar Foam::TDACChemistryModel<ReactionThermo, ThermoType>::solve
(
    const scalar deltaT
)
{
    // Don't allow the time-step to change more than a factor of 2
    return min
    (
        this->solve<UniformField<scalar>>(UniformField<scalar>(deltaT)),
        2*deltaT
    );
}


template<class ReactionThermo, class ThermoType>
Foam::scalar Foam::TDACChemistryModel<ReactionThermo, ThermoType>::solve
(
    const scalarField& deltaT
)
{
    return this->solve<scalarField>(deltaT);
}


template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::
setTabulationResultsAdd
(
    const label celli
)
{
    tabulationResults_[celli] = 0.0;
}


template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::
setTabulationResultsGrow(const label celli)
{
    tabulationResults_[celli] = 1.0;
}


template<class ReactionThermo, class ThermoType>
void Foam::TDACChemistryModel<ReactionThermo, ThermoType>::
setTabulationResultsRetrieve
(
    const label celli
)
{
    tabulationResults_[celli] = 2.0;
}


// ************************************************************************* //
