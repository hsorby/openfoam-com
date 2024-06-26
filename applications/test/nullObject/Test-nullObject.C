/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2014 OpenFOAM Foundation
    Copyright (C) 2019-2020 OpenCFD Ltd.
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

Application
    Test-nullObject

Description
    Tests of nullObject

\*---------------------------------------------------------------------------*/

#include "primitives/nullObject/nullObject.H"
#include "containers/Lists/List/List.H"
#include "containers/HashTables/HashSet/HashSet.H"
#include "meshes/meshShapes/face/faceList.H"
#include "meshes/primitiveShapes/point/pointField.H"
#include "db/IOstreams/IOstreams.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

using namespace Foam;

class SimpleClass
{
public:

    //- Default construct
    SimpleClass() {}
};


template<class T>
void printInfo(const UList<T>& list)
{
    std::cout
        << nl
        << "List : addr: " << name(&list)
        << " (null: " << isNull(list) << ")" << nl
        << "    size: " << list.size() << " empty: " << list.empty() << nl
        << "    data: " << name(list.cdata())
        << " begin=" << name(list.begin())
        << " end=" << name(list.end()) << nl;

    Info<< list << nl;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main()
{
    // Test pointer and reference to a class

    SimpleClass* ptrToClass = new SimpleClass;
    SimpleClass& refToClass(*ptrToClass);

    std::cout
        << "nullObject addr=" << name(&(nullObjectPtr)) << nl
        << "  sizeof(nullObject) = " << sizeof(NullObject::nullObject) << nl
        << "  sizeof(void*) = " << sizeof(void*) << nl
        << "  sizeof(labelList) = " << sizeof(labelList) << nl
        << "  sizeof(wordHashSet) = " << sizeof(wordHashSet) << nl << nl;

    std::cout
        << "nullObject" << nl
        << "  pointer:" << name(nullObjectPtr->pointer()) << nl
        << "  value:"   << nullObjectPtr->value() << nl << nl;

    if (notNull(ptrToClass))
    {
        Info<< "Pass: ptrToClass is not null" << nl;
    }
    else
    {
        Info<< "FAIL: refToClass is null" << nl;
    }

    if (notNull(refToClass))
    {
        Info<< "Pass: refToClass is not null" << nl;
    }
    else
    {
        Info<< "FAIL: refToClass is null" << nl;
    }


    // Test pointer and reference to the nullObject

    const SimpleClass* ptrToNull(NullObjectPtr<SimpleClass>());
    const SimpleClass& refToNull(*ptrToNull);

    if (isNull(ptrToNull))
    {
        Info<< "Pass: ptrToNull is null" << nl;
    }
    else
    {
        Info<< "FAIL: ptrToNull is not null" << nl;
    }

    if (isNull(refToNull))
    {
        Info<< "Pass: refToNull is null" << nl;
    }
    else
    {
        Info<< "FAIL: refToNull is not null" << nl;
    }

    // Clean-up
    delete ptrToClass;


    // Test List casting
    {
        labelList list1;
        labelList list2({1, 2, 3});

        printInfo(list1);
        printInfo(list2);
        printInfo(labelList::null());

        printInfo(faceList::null());
        printInfo(pointField::null());
    }

    // Test swallow assigment (like std::ignore)
    // Looks pretty ugly though!

    NullObject::nullObject = "hello world";
    NullObject::nullObject = labelList({1, 2, 3});

    Info<< nl;

    return 0;
}

// ************************************************************************* //
