/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2017 OpenFOAM Foundation
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
    Foam::fv::VoFSolidificationMeltingSource

Description
    Solidification and melting model for VoF simulations.

    The presence of the solid phase in the flow field is incorporated into the
    model as a momentum porosity contribution; the energy associated with the
    phase change is added as an enthalpy contribution.  The solid fraction as a
    function of temperature \c alphaSolidT is specified as a Foam::Function1.

    The model writes the field \c alpha[01].solid which can be visualised to to
    show the solid distribution.

Usage
    Example usage:
    \verbatim
    VoFSolidificationMeltingSource1
    {
        type            VoFSolidificationMeltingSource;
        active          yes;

        selectionMode   cellZone;
        cellZone        solidZone;

        alphaSolidT     table
        (
            (330 1)
            (335 0)
        );

        L               334000;
    }
    \endverbatim

    Where:
    \table
        Property     | Description             | Required    | Default value
        alphaSolidT  | Solid fraction as function of temperature | yes |
        L            | Latent heat of fusion [J/kg] | yes    |
        relax        | Relaxation coefficient [0-1] | no     | 0.9
        Cu           | Model coefficient       | no          | 100000
        q            | Model coefficient       | no          | 0.001
    \endtable

See also
    Foam::fv::solidificationMeltingSource
    Foam::Function1

SourceFiles
    VoFSolidificationMeltingSource.C
    VoFSolidificationMeltingSourceIO.C

\*---------------------------------------------------------------------------*/

#ifndef VoFSolidificationMeltingSource_H
#define VoFSolidificationMeltingSource_H

#include "fvMesh/fvMesh.H"
#include "fields/volFields/volFields.H"
#include "cellSetOption/cellSetOption.H"
#include "Function1.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                 Class VoFSolidificationMeltingSource Declaration
\*---------------------------------------------------------------------------*/

class VoFSolidificationMeltingSource
:
    public fv::cellSetOption
{
    // Private data

        //- Solid fraction as a function of temperature
        autoPtr<Function1<scalar>> alphaSolidT_;

        //- Latent heat of fusion [J/kg]
        dimensionedScalar L_;

        //- Phase fraction under-relaxation coefficient
        scalar relax_;

        //- Mushy region momentum sink coefficient [1/s]; default = 10^5
        scalar Cu_;

        //- Coefficient used in porosity calc - default = 0.001
        scalar q_;

        //- Solid phase fraction
        volScalarField alphaSolid_;

        //- Current time index (used for updating)
        label curTimeIndex_;


    // Private Member Functions

        //- Return the name of the solid phase fraction
        word alphaSolidName() const;

        //- Update the model
        void update();

        //- Helper function to apply to the energy equation
        template<class RhoFieldType>
        void apply(const RhoFieldType& rho, fvMatrix<scalar>& eqn);

        //- No copy construct
        VoFSolidificationMeltingSource
        (
            const VoFSolidificationMeltingSource&
        ) = delete;

        //- No copy assignment
        void operator=(const VoFSolidificationMeltingSource&) = delete;


public:

    //- Runtime type information
    TypeName("VoFSolidificationMeltingSource");


    // Constructors

        //- Construct from explicit source name and mesh
        VoFSolidificationMeltingSource
        (
            const word& sourceName,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    // Member Functions

        // Add explicit and implicit contributions

            //- Add explicit contribution to enthalpy equation
            virtual void addSup(fvMatrix<scalar>& eqn, const label fieldi);

            //- Add implicit contribution to momentum equation
            virtual void addSup(fvMatrix<vector>& eqn, const label fieldi);


        // Add explicit and implicit contributions to compressible equation

            //- Add explicit contribution to compressible enthalpy equation
            virtual void addSup
            (
                const volScalarField& rho,
                fvMatrix<scalar>& eqn,
                const label fieldi
            );

            //- Add implicit contribution to compressible momentum equation
            virtual void addSup
            (
                const volScalarField& rho,
                fvMatrix<vector>& eqn,
                const label fieldi
            );


        // IO

            //- Read source dictionary
            virtual bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "VoFSolidificationMeltingSource/VoFSolidificationMeltingSourceTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
