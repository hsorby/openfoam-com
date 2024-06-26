/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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
    Foam::DiagonalSolver

Description
    Foam::DiagonalSolver

SourceFiles
    DiagonalSolver.C

\*---------------------------------------------------------------------------*/

#ifndef Foam_DiagonalSolver_H
#define Foam_DiagonalSolver_H

#include "matrices/LduMatrixCaseDir/LduMatrix/LduMatrixPascal.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class DiagonalSolver Declaration
\*---------------------------------------------------------------------------*/

template<class Type, class DType, class LUType>
class DiagonalSolver
:
    public LduMatrix<Type, DType, LUType>::solver
{
public:

    // Generated Methods

        //- No copy construct
        DiagonalSolver(const DiagonalSolver&) = delete;

        //- No copy assignment
        void operator=(const DiagonalSolver&) = delete;


    //- Runtime type information
    TypeName("diagonal");


    // Constructors

        //- Construct from matrix
        DiagonalSolver
        (
            const word& fieldName,
            const LduMatrix<Type, DType, LUType>& matrix,
            const dictionary& solverDict
        );


    // Member Functions

        //- Read and reset the solver parameters from the given dictionary
        void read(const dictionary& solverDict);

        //- Solve the matrix with this solver
        virtual SolverPerformance<Type> solve(Field<Type>& psi) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "matrices/LduMatrixCaseDir/Solvers/DiagonalSolver/DiagonalSolverPascal.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
