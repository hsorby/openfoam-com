/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2013 OpenFOAM Foundation
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
    Foam::profileModelList

Description
    Base class for profile models

SourceFiles
    profileModelList.C

\*---------------------------------------------------------------------------*/

#ifndef profileModelList_H
#define profileModelList_H

#include "containers/PtrLists/PtrList/PtrList.H"
#include "sources/derived/rotorDiskSource/profileModel/profileModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class profileModelList Declaration
\*---------------------------------------------------------------------------*/

class profileModelList
:
    public PtrList<profileModel>
{
protected:

    // Protected Data

        //- Dictionary
        const dictionary dict_;


public:

    //- Constructor
    profileModelList(const dictionary& dict, const bool readFields = true);

    //- Destructor
    ~profileModelList() = default;


    // Member Functions

        //- Set blade->profile addressing
        void connectBlades
        (
            const List<word>& names,
            List<label>& addr
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
