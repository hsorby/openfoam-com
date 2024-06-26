/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016 OpenFOAM Foundation
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
    Foam::RBD::joints::composite

Group
    grpRigidBodyDynamicsJoints

Description
    Prismatic joint for translation along the specified arbitrary axis.

    Reference:
    \verbatim
        Featherstone, R. (2008).
        Rigid body dynamics algorithms.
        Springer.
        Chapter 4.
    \endverbatim

SourceFiles
    composite.C

\*---------------------------------------------------------------------------*/

#ifndef RBD_joints_composite_H
#define RBD_joints_composite_H

#include "joints/joint/joint.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace RBD
{

// Forward declaration of classes
class rigidBodyModel;

namespace joints
{

/*---------------------------------------------------------------------------*\
                         Class composite Declaration
\*---------------------------------------------------------------------------*/

class composite
:
    public PtrList<joint>,
    public joint
{
    // Private member functions

        //- Set the properties of the last joint following construction
        //  of the body containing the joint
        void setLastJoint();


public:

    //- Runtime type information
    TypeName("composite");

    //- Allow the rigidBodyModel class to set the last joint state
    friend class Foam::RBD::rigidBodyModel;


    // Constructors

        //- Construct for given PtrList<joint>
        composite(const PtrList<joint>& joints);

        //- Construct for given model from dictionary
        composite(const dictionary& dict);

        //- Clone this joint
        virtual autoPtr<joint> clone() const;


    //- Destructor
    virtual ~composite();


    // Member Functions

        //- Update the model state for this joint
        virtual void jcalc
        (
            joint::XSvc& J,
            const scalarField& q,
            const scalarField& qDot
        ) const;

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace joints
} // End namespace RBD
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
