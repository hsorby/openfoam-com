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

\*---------------------------------------------------------------------------*/

#include "thermophysicalPropertiesSelector/thermophysicalPropertiesSelector.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class ThermophysicalProperties>
Foam::thermophysicalPropertiesSelector<ThermophysicalProperties>::
thermophysicalPropertiesSelector
(
    const word& name
)
:
    propertiesPtr_(ThermophysicalProperties::New(name))
{}


template<class ThermophysicalProperties>
Foam::thermophysicalPropertiesSelector<ThermophysicalProperties>::
thermophysicalPropertiesSelector
(
    const dictionary& dict
)
{
    const word name(dict.first()->keyword());

    // Handle sub-dictionary or primitive entry

    const dictionary* subDictPtr = dict.findDict(name);

    if (subDictPtr)
    {
        propertiesPtr_ = ThermophysicalProperties::New(*subDictPtr);
    }
    else
    {
        propertiesPtr_ = ThermophysicalProperties::New(name);
    }
}


// ************************************************************************* //
