/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2013 OpenFOAM Foundation
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
    Foam::cyclicAMIGAMGInterfaceField

Description
    GAMG agglomerated cyclic interface field.

SourceFiles
    cyclicAMIGAMGInterfaceField.C

\*---------------------------------------------------------------------------*/

#ifndef cyclicAMIGAMGInterfaceField_H
#define cyclicAMIGAMGInterfaceField_H

#include "matrices/lduMatrix/solvers/GAMG/interfaceFields/GAMGInterfaceField/GAMGInterfaceField.H"
#include "AMIInterpolation/GAMG/interfaces/cyclicAMIGAMGInterface/cyclicAMIGAMGInterface.H"
#include "AMIInterpolation/patches/cyclicAMI/cyclicAMILduInterfaceField/cyclicAMILduInterfaceField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                 Class cyclicAMIGAMGInterfaceField Declaration
\*---------------------------------------------------------------------------*/

class cyclicAMIGAMGInterfaceField
:
    public GAMGInterfaceField,
    virtual public cyclicAMILduInterfaceField
{
    // Private data

        //- Local reference cast into the cyclic interface
        const cyclicAMIGAMGInterface& cyclicAMIInterface_;

        //- Is the transform required
        bool doTransform_;

        //- Rank of component for transformation
        int rank_;


        // Sending and receiving (distributed AMI)

            //- Current range of send requests (non-blocking)
            mutable labelRange sendRequests_;

            //- Current range of recv requests (non-blocking)
            mutable labelRange recvRequests_;

            //- Scalar send buffers
            mutable PtrList<List<solveScalar>> scalarSendBufs_;

            //- Scalar receive buffers
            mutable PtrList<List<solveScalar>> scalarRecvBufs_;


    // Private Member Functions

        //- No copy construct
        cyclicAMIGAMGInterfaceField
        (
            const cyclicAMIGAMGInterfaceField&
        ) = delete;

        //- No copy assignment
        void operator=(const cyclicAMIGAMGInterfaceField&) = delete;


public:

    //- Runtime type information
    TypeName("cyclicAMI");


    // Constructors

        //- Construct from GAMG interface and fine level interface field
        cyclicAMIGAMGInterfaceField
        (
            const GAMGInterface& GAMGCp,
            const lduInterfaceField& fineInterfaceField
        );

        //- Construct from GAMG interface and fine level interface field
        cyclicAMIGAMGInterfaceField
        (
            const GAMGInterface& GAMGCp,
            const bool doTransform,
            const int rank
        );

        //- Construct from GAMG interface and Istream
        cyclicAMIGAMGInterfaceField
        (
            const GAMGInterface& GAMGCp,
            Istream& is
        );

        //- Construct from GAMG interface and local and remote fields
        cyclicAMIGAMGInterfaceField
        (
            const GAMGInterface& GAMGCp,
            const lduInterfaceField& local,
            const UPtrList<lduInterfaceField>& other
        );

        //- Construct by assembling and return a clone.
        virtual autoPtr<GAMGInterfaceField> clone
        (
            const GAMGInterface& GAMGCp,
            const UPtrList<lduInterfaceField>& other
        ) const
        {
            return autoPtr<GAMGInterfaceField>
            (
                new cyclicAMIGAMGInterfaceField
                (
                    GAMGCp,
                    *this,      // local field
                    other       // other fields
                )
            );
        }


    //- Destructor
    virtual ~cyclicAMIGAMGInterfaceField() = default;


    // Member Functions

        // Access

            //- Return size
            label size() const
            {
                return cyclicAMIInterface_.size();
            }


        // Interface matrix update

            //- Are all (receive) data available?
            virtual bool ready() const;

            //- Initialise neighbour matrix update
            virtual void initInterfaceMatrixUpdate
            (
                solveScalarField& result,
                const bool add,
                const lduAddressing& lduAddr,
                const label patchId,
                const solveScalarField& psiInternal,
                const scalarField& coeffs,
                const direction cmpt,
                const Pstream::commsTypes commsType
            ) const;

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                solveScalarField& result,
                const bool add,
                const lduAddressing& lduAddr,
                const label patchId,
                const solveScalarField& psiInternal,
                const scalarField& coeffs,
                const direction cmpt,
                const Pstream::commsTypes commsType
            ) const;


        //- Cyclic interface functions

            //- Does the interface field perform the transformation
            virtual bool doTransform() const
            {
                return doTransform_;
            }

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const
            {
                return cyclicAMIInterface_.forwardT();
            }

            //- Return neighbour-cell transformation tensor
            virtual const tensorField& reverseT() const
            {
                return cyclicAMIInterface_.reverseT();
            }

            //- Return rank of component for transform
            virtual int rank() const
            {
                return rank_;
            }


        // I/O

            //- Write to stream
            virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
