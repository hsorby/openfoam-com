/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2018 OpenFOAM Foundation
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

Class
    Foam::WLFTransport

Description
    Transport package using the Williams-Landel-Ferry model.

    Templated into a given thermodynamics package (needed for thermal
    conductivity).

    Dynamic viscosity [kg/m.s]
    \f[
        \mu = \mu_0 \exp \left(\frac{-C_1 ( T - T_r )}{C_2 + T - T_r}\right)
    \f]

    References:
    \verbatim
        Williams, M. L., Landel, R. F., & Ferry, J. D. (1955).
        The temperature dependence of relaxation mechanisms
        in amorphous polymers and other glass-forming liquids.
        Journal of the American Chemical society, 77(14), 3701-3707.
    \endverbatim

SourceFiles
    WLFTransportI.H
    WLFTransport.C

\*---------------------------------------------------------------------------*/

#ifndef WLFTransport_H
#define WLFTransport_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward Declarations

template<class Thermo> class WLFTransport;

template<class Thermo>
inline WLFTransport<Thermo> operator+
(
    const WLFTransport<Thermo>&,
    const WLFTransport<Thermo>&
);

template<class Thermo>
inline WLFTransport<Thermo> operator*
(
    const scalar,
    const WLFTransport<Thermo>&
);

template<class Thermo>
Ostream& operator<<
(
    Ostream&,
    const WLFTransport<Thermo>&
);


/*---------------------------------------------------------------------------*\
                    Class WLFTransport Declaration
\*---------------------------------------------------------------------------*/

template<class Thermo>
class WLFTransport
:
    public Thermo
{
    // Private Data

        //- Dynamic viscosity at the reference temperature [Pa.s]
        scalar mu0_;

        //- Reference temperature [T]
        scalar Tr_;

        //- WLF coefficient 1 []
        scalar C1_;

        //- WLF coefficient 2 [T]
        scalar C2_;

        //- Reciprocal Prandtl Number [-]
        scalar rPr_;


    // Private Member Functions

        //- Read coefficient from "transport" sub-dictionary
        inline scalar readCoeff(const word& key, const dictionary& dict)
        {
            return dict.subDict("transport").get<scalar>(key);
        }


public:

    // Generated Methods: copy construct, copy assignment


    // Constructors

        //- Construct as named copy
        inline WLFTransport(const word&, const WLFTransport&);

        //- Construct from dictionary
        explicit WLFTransport(const dictionary& dict);

        //- Construct and return a clone
        inline autoPtr<WLFTransport> clone() const;

        // Selector from dictionary
        inline static autoPtr<WLFTransport> New(const dictionary& dict);


    // Member Functions

        //- Return the instantiated type name
        static word typeName()
        {
            return "WLF<" + Thermo::typeName() + '>';
        }

        //- Dynamic viscosity [kg/ms]
        inline scalar mu(const scalar p, const scalar T) const;

        //- Thermal conductivity [W/mK]
        inline scalar kappa(const scalar p, const scalar T) const;

        //- Thermal diffusivity of enthalpy [kg/ms]
        inline scalar alphah(const scalar p, const scalar T) const;

        // Species diffusivity
        // inline scalar D(const scalar p, const scalar T) const;

        //- Write to Ostream
        void write(Ostream& os) const;


    // Member Operators

        inline void operator+=(const WLFTransport&);

        inline void operator*=(const scalar);


    // Friend Operators

        friend WLFTransport operator+ <Thermo>
        (
            const WLFTransport&,
            const WLFTransport&
        );

        friend WLFTransport operator* <Thermo>
        (
            const scalar,
            const WLFTransport&
        );


    // IOstream Operators

        friend Ostream& operator<< <Thermo>
        (
            Ostream&,
            const WLFTransport&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "transport/WLF/WLFTransportI.H"

#ifdef NoRepository
    #include "transport/WLF/WLFTransport.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
