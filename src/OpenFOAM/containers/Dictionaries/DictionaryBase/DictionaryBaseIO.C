/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "containers/Dictionaries/DictionaryBase/DictionaryBase.H"
#include "db/IOstreams/IOstreams.H"

// * * * * * * * * * * * * * * Ostream Operator  * * * * * * * * * * * * * * //

template<class IDLListType, class T>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const DictionaryBase<IDLListType, T>& dict
)
{
    for (auto iter = dict.cbegin(); iter != dict.cend(); ++iter)
    {
        os << *iter;

        // Check stream before going to next entry.
        if (!os.good())
        {
            Info
                << "operator<<(Ostream&, const DictionaryBase&) : "
                << "Can't write entry for DictionaryBase"
                << endl;

            break;
        }
    }

    return os;
}


// ************************************************************************* //
