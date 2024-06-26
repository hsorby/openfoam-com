/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2022 OpenCFD Ltd.
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

#include "specie/specie.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Thermo>
inline Foam::tabulatedSolidTransport<Thermo>::tabulatedSolidTransport
(
    const Thermo& t,
    const nonUniformTable& kappa
)
:
    Thermo(t),
    kappa_(kappa)
{}


template<class Thermo>
inline Foam::tabulatedSolidTransport<Thermo>::tabulatedSolidTransport
(
    const word& name,
    const tabulatedSolidTransport& pt
)
:
    Thermo(name, pt),
    kappa_(pt.kappa_)
{}


template<class Thermo>
inline Foam::autoPtr<Foam::tabulatedSolidTransport<Thermo>>
Foam::tabulatedSolidTransport<Thermo>::clone() const
{
    return autoPtr<tabulatedSolidTransport<Thermo>>::New(*this);
}


template<class Thermo>
inline Foam::autoPtr<Foam::tabulatedSolidTransport<Thermo>>
Foam::tabulatedSolidTransport<Thermo>::New(const dictionary& dict)
{
    return autoPtr<tabulatedSolidTransport<Thermo>>::New(dict);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Thermo>
inline Foam::scalar Foam::tabulatedSolidTransport<Thermo>::mu
(
    const scalar p,
    const scalar T
) const
{
    NotImplemented;
    return 0;
}


template<class Thermo>
inline Foam::scalar Foam::tabulatedSolidTransport<Thermo>::kappa
(
    const scalar p,
    const scalar T
) const
{
    return kappa_.f(p, T);
}


template<class Thermo>
inline Foam::vector Foam::tabulatedSolidTransport<Thermo>::Kappa
(
    const scalar p,
    const scalar T
) const
{
    const scalar kappa(kappa_.f(p, T));
    return vector(kappa, kappa, kappa);
}


template<class Thermo>
inline Foam::scalar Foam::tabulatedSolidTransport<Thermo>::alphah
(
    const scalar p,
    const scalar T
) const
{
    return kappa(p, T)/this->Cp(p, T);
}


// ************************************************************************* //
