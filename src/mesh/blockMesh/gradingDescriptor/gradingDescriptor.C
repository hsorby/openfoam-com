/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2015 OpenFOAM Foundation
    Copyright (C) 2019-2021 OpenCFD Ltd.
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

#include "gradingDescriptor/gradingDescriptor.H"
#include "db/IOstreams/IOstreams/IOstream.H"
#include "db/IOstreams/token/token.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::gradingDescriptor::gradingDescriptor()
:
    blockFraction_(1),
    nDivFraction_(1),
    expansionRatio_(1)
{}


Foam::gradingDescriptor::gradingDescriptor
(
    const scalar blockFraction,
    const scalar nDivFraction,
    const scalar expansionRatio
)
:
    blockFraction_(blockFraction),
    nDivFraction_(nDivFraction),
    expansionRatio_(expansionRatio)
{
    correct();
}


Foam::gradingDescriptor::gradingDescriptor
(
    const scalar expansionRatio
)
:
    blockFraction_(1),
    nDivFraction_(1),
    expansionRatio_(expansionRatio)
{
    correct();
}


Foam::gradingDescriptor::gradingDescriptor(Istream& is)
{
    is >> *this;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::gradingDescriptor::correct()
{
    if (expansionRatio_ < 0)
    {
        expansionRatio_ = 1.0/(-expansionRatio_);
    }
}


Foam::gradingDescriptor Foam::gradingDescriptor::inv() const
{
    return gradingDescriptor
    (
        blockFraction_,
        nDivFraction_,
        1.0/expansionRatio_
    );
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

bool Foam::gradingDescriptor::operator==(const gradingDescriptor& gd) const
{
    return
        equal(blockFraction_, gd.blockFraction_)
     && equal(nDivFraction_, gd.nDivFraction_)
     && equal(expansionRatio_, gd.expansionRatio_);
}


bool Foam::gradingDescriptor::operator!=(const gradingDescriptor& gd) const
{
    return !operator==(gd);
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

Foam::Istream& Foam::operator>>(Istream& is, gradingDescriptor& gd)
{
    // Examine next token
    token t(is);

    if (t.isNumber())
    {
        gd.blockFraction_ = 1.0;
        gd.nDivFraction_ = 1.0;
        gd.expansionRatio_ = t.number();
    }
    else if (t.isPunctuation(token::BEGIN_LIST))
    {
        is >> gd.blockFraction_ >> gd.nDivFraction_ >> gd.expansionRatio_;
        is.readEnd("gradingDescriptor");
    }

    gd.correct();

    is.check(FUNCTION_NAME);
    return is;
}


Foam::Ostream& Foam::operator<<(Ostream& os, const gradingDescriptor& gd)
{
    if (equal(gd.blockFraction_, 1))
    {
        os  << gd.expansionRatio_;
    }
    else
    {
        os  << token::BEGIN_LIST
            << gd.blockFraction_ << token::SPACE
            << gd.nDivFraction_ << token::SPACE
            << gd.expansionRatio_
            << token::END_LIST;
    }

    return os;
}


// ************************************************************************* //
