/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2013 OpenFOAM Foundation
    Copyright (C) 2020-2023 OpenCFD Ltd.
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

#include "meshes/Identifiers/patch/patchIdentifier.H"
#include "db/dictionary/dictionary.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patchIdentifier::patchIdentifier()
:
    name_(),
    index_(0)
{}


Foam::patchIdentifier::patchIdentifier
(
    const word& name,
    const label index
)
:
    name_(name),
    index_(index)
{}


Foam::patchIdentifier::patchIdentifier
(
    const word& name,
    const label index,
    const word& physicalType,
    const wordList& inGroups
)
:
    name_(name),
    index_(index),
    physicalType_(physicalType),
    inGroups_(inGroups)
{}


Foam::patchIdentifier::patchIdentifier
(
    const word& name,
    const dictionary& dict,
    const label index
)
:
    patchIdentifier(name, index)
{
    dict.readIfPresent("physicalType", physicalType_);
    dict.readIfPresent("inGroups", inGroups_);
}


Foam::patchIdentifier::patchIdentifier
(
    const patchIdentifier& ident,
    const label newIndex
)
:
    patchIdentifier(ident)
{
    if (newIndex >= 0)
    {
        index_ = newIndex;
    }
}


Foam::patchIdentifier::patchIdentifier
(
    patchIdentifier&& ident,
    const label newIndex
)
:
    patchIdentifier(std::move(ident))
{
    if (newIndex >= 0)
    {
        index_ = newIndex;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::patchIdentifier::removeGroup(const word& name)
{
    label idx = name.empty() ? -1 : inGroups_.find(name);

    if (idx >= 0)
    {
        for (label i = idx + 1; i < inGroups_.size(); ++i)
        {
            if (inGroups_[i] != name)
            {
                inGroups_[idx] = std::move(inGroups_[i]);
                ++idx;
            }
        }
        inGroups_.resize(idx);
    }
}


void Foam::patchIdentifier::write(Ostream& os) const
{
    if (!physicalType_.empty())
    {
        os.writeEntry("physicalType", physicalType_);
    }

    if (!inGroups_.empty())
    {
        os.writeKeyword("inGroups");
        inGroups_.writeList(os, 0);  // Flat output
        os.endEntry();
    }
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

Foam::Ostream& Foam::operator<<(Ostream& os, const patchIdentifier& ident)
{
    ident.write(os);
    os.check(FUNCTION_NAME);
    return os;
}


// ************************************************************************* //
