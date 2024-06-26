/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2012 OpenFOAM Foundation
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
    Foam::subCycle

Description
    Perform a subCycleTime on a field

\*---------------------------------------------------------------------------*/

#ifndef subCycle_H
#define subCycle_H

#include "db/Time/subCycleTime.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class subCycleField Declaration
\*---------------------------------------------------------------------------*/

template<class GeometricField>
class subCycleField
{
    // Private data

        //- Reference to the field being sub-cycled
        GeometricField& gf_;

        //- Reference to the field old-time field being sub-cycled
        //  Needed to avoid calls to oldTime() which may cause
        //  unexpected updates of the old-time field
        GeometricField& gf0_;

        //- Copy of the "real" old-time value of the field
        tmp<GeometricField> gf_0_;

        GeometricField& gf00_;

        //- Copy of the "real" old-old-time value of the field
        tmp<GeometricField> gf_00_;


public:

    // Constructors

        //- Construct field and number of sub-cycles
        subCycleField(GeometricField& gf)
        :
            gf_(gf),
            gf0_(gf.oldTime()),
            gf00_(gf.oldTime().oldTime())
        {
            {
                gf_0_ = GeometricField::New(gf0_.name() + "_", gf0_);
                gf_00_ = GeometricField::New(gf00_.name() + "_", gf00_);
            }
        }


    //- Destructor
    ~subCycleField()
    {
        if (gf_0_.valid())
        {
            // Reset the old-time field
            gf0_ = gf_0_;

            gf00_ = gf_00_;

        // Correct the time index of the field to correspond to the global time
            gf_.timeIndex() = gf_.time().timeIndex();
        }
    }


    //- Correct the time index of the field to correspond to
    //  the sub-cycling time.
    //
    //  The time index is incremented to protect the old-time value from
    //  being updated at the beginning of the time-loop in the case of
    //  outer iteration
    void updateTimeIndex()
    {
        gf_.timeIndex() = gf_.time().timeIndex() + 1;
        gf0_.timeIndex() = gf0_.time().timeIndex() + 1;
        gf00_.timeIndex() = gf00_.time().timeIndex() + 1;
    }
};


/*---------------------------------------------------------------------------*\
                          Class subCycle Declaration
\*---------------------------------------------------------------------------*/

template<class GeometricField>
class subCycle
:
    public subCycleField<GeometricField>,
    public subCycleTime
{
    // Private Member Functions

        //- No copy construct
        subCycle(const subCycle<GeometricField>&) = delete;

        //- No copy assignment
        void operator=(const subCycle<GeometricField>&) = delete;


public:

    // Constructors

        //- Construct field and number of sub-cycles
        subCycle(GeometricField& gf, const label nCycles)
        :
            subCycleField<GeometricField>(gf),
            subCycleTime(const_cast<Time&>(gf.time()), nCycles)
        {
            // Update the field time index to correspond to the sub-cycle time
            this->updateTimeIndex();
        }


    //- Destructor
    //  End the subCycleTime, which restores the time state
    ~subCycle()
    {
        endSubCycle();
    }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
