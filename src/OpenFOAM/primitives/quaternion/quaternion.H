/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2019-2023 OpenCFD Ltd.
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
    Foam::quaternion

Description
    Quaternion class used to perform rotations in 3D space.

SourceFiles
    quaternionI.H
    quaternion.C

\*---------------------------------------------------------------------------*/

#ifndef Foam_quaternion_H
#define Foam_quaternion_H

#include "primitives/Scalar/scalar/scalar.H"
#include "primitives/Vector/floats/vector.H"
#include "primitives/Tensor/floats/tensor.H"
#include "primitives/strings/word/word.H"
#include "primitives/enums/Enum.H"
#include "include/stdFoam.H"  // For deprecation macros

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class quaternion Declaration
\*---------------------------------------------------------------------------*/

class quaternion
{
    // Private Data

        //- Scalar part of the quaternion ( = cos(theta/2) for rotation)
        scalar w_;

        //- Vector part of the quaternion ( = axis of rotation)
        vector v_;


        //- Multiply vector v by quaternion as if v is a pure quaternion
        inline quaternion mulq0v(const vector& v) const;

        //- Conversion of two-axis rotation components into Euler-angles
        inline static vector twoAxes
        (
            const scalar t11,
            const scalar t12,
            const scalar c2,
            const scalar t31,
            const scalar t32
        );

        //- Conversion of three-axis rotation components into Euler-angles
        inline static vector threeAxes
        (
            const scalar t11,
            const scalar t12,
            const scalar s2,
            const scalar t31,
            const scalar t32
        );


public:

    // Public Types

        //- Component type
        typedef scalar cmptType;

        //- Magnitude type
        typedef scalar magType;

        //- Euler-angle rotation order
        enum eulerOrder : unsigned char
        {
            // Proper Euler angles
            XZX, XYX, YXY, YZY, ZYZ, ZXZ,

            // Tait-Bryan angles
            XZY, XYZ, YXZ, YZX, ZYX, ZXY,

            // Aliases
            ROLL_PITCH_YAW = XYZ,
            YAW_PITCH_ROLL = ZYX,
        };

        //- The names for Euler-angle and Tait-Bryan angles,
        //- including "rollPitchYaw" and "yawPitchRoll" aliases
        static const Enum<eulerOrder> eulerOrderNames;


    // Member Constants

        //- Rank of quaternion is 1
        static constexpr direction rank = 1;


    // Static Data Members

        static constexpr const char* const typeName = "quaternion";

        static const quaternion zero;
        static const quaternion I;


    // Generated Methods

        //- Default construct
        quaternion() = default;

        //- Copy construct
        quaternion(const quaternion&) = default;

        //- Copy assignment
        quaternion& operator=(const quaternion&) = default;


    // Constructors

        //- Construct initialized to zero
        inline quaternion(const Foam::zero);

        //- Construct given scalar and vector parts
        inline quaternion(const scalar w, const vector& v);

        //- Construct rotation quaternion given direction d and angle theta
        inline quaternion(const vector& d, const scalar theta);

        //- Construct a rotation quaternion given direction d
        //- and cosine angle cosTheta and flag if d is normalised
        inline quaternion
        (
            const vector& d,
            const scalar cosTheta,
            const bool isNormalised
        );

        //- Construct a real quaternion from the given scalar part,
        //- the vector part = zero
        inline explicit quaternion(const scalar w);

        //- Construct a pure imaginary quaternion given the vector part,
        //- the scalar part = 0
        inline explicit quaternion(const vector& v);

        //- Return the unit quaternion (versor) from the given vector
        //- (w = sqrt(1 - |sqr(v)|))
        static inline quaternion unit(const vector& v);

        //- Construct from three Euler rotation angles
        inline quaternion(const eulerOrder order, const vector& angles);

        //- Construct from a rotation tensor
        inline explicit quaternion(const tensor& rotationTensor);

        //- Construct from Istream
        explicit quaternion(Istream& is);


    // Member Functions

    // Access

        //- Scalar part of the quaternion ( = cos(theta/2) for rotation)
        inline scalar w() const noexcept;

        //- Vector part of the quaternion ( = axis of rotation)
        inline const vector& v() const noexcept;

        //- The rotation tensor corresponding to the quaternion
        inline tensor R() const;

        //- Return the Euler rotation angles corresponding to the
        //- specified rotation order
        inline vector eulerAngles(const eulerOrder order) const;


    // Edit

        //- Scalar part of the quaternion ( = cos(theta/2) for rotation)
        inline scalar& w() noexcept;

        //- Vector part of the quaternion ( = axis of rotation)
        inline vector& v() noexcept;

        //- Inplace normalise the quaternion by its magnitude
        //  For small magnitudes (less than ROOTVSMALL) set to zero.
        inline quaternion& normalise(const scalar tol = ROOTVSMALL);


    // Transform

       //- Rotate the given vector
       inline vector transform(const vector& v) const;

       //- Rotate the given vector anti-clockwise
       inline vector invTransform(const vector& v) const;

       //- Rotate the given quaternion (and normalise)
       inline quaternion transform(const quaternion& q) const;

       //- Rotate the given quaternion anti-clockwise (and normalise)
       inline quaternion invTransform(const quaternion& q) const;


    // Member Operators

        inline void operator+=(const quaternion& q);
        inline void operator-=(const quaternion& q);
        inline void operator*=(const quaternion& q);
        inline void operator/=(const quaternion& q);

        //- Change scalar portion only
        inline void operator=(const scalar s);

        //- Change vector portion only
        inline void operator=(const vector& v);

        //- Assign scalar and vector to zero
        inline void operator=(const Foam::zero);

        inline void operator*=(const scalar s);
        inline void operator/=(const scalar s);


    // Housekeeping

        //- Inplace normalise the quaternion by its magnitude
        FOAM_DEPRECATED_FOR(2022-04, "normalise() method")
        void normalize() { this->normalise(); }

        //- Return the quaternion normalised by its magnitude
        FOAM_DEPRECATED_FOR(2022-04, "Use Foam::normalised() function")
        quaternion normalized() const
        {
            quaternion q(*this);
            q.normalise();
            return q;
        }
};


// * * * * * * * * * * * * * * * * * Traits  * * * * * * * * * * * * * * * * //

//- Contiguous data for quaternion
template<> struct is_contiguous<quaternion> : std::true_type {};

//- Contiguous scalar data for quaternion
template<> struct is_contiguous_scalar<quaternion> : std::true_type {};


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

inline scalar magSqr(const quaternion& q);
inline scalar mag(const quaternion& q);

//- Return the conjugate of the given quaternion
inline quaternion conjugate(const quaternion& q);

//- Return the normalised (unit) quaternion of the given quaternion
inline quaternion normalised(const quaternion& q);

//- Return the normalised (unit) quaternion of the given quaternion
FOAM_DEPRECATED_FOR(2022-04, "Use Foam::normalised() function")
inline quaternion normalize(const quaternion& q) { return Foam::normalised(q); }

//- Return the inverse of the given quaternion
inline quaternion inv(const quaternion& q);

//- Return a string representation of a quaternion
word name(const quaternion& q);

//- Spherical linear interpolation of quaternions
quaternion slerp
(
    const quaternion& qa,
    const quaternion& qb,
    const scalar t
);

//- Simple weighted average with sign change
quaternion average
(
    const UList<quaternion>& qs,
    const UList<scalar> w
);

//- Exponent of a quaternion
quaternion exp(const quaternion& q);

//- Power of a quaternion
quaternion pow(const quaternion& q, const label power);

//- Power of a quaternion
quaternion pow(const quaternion& q, const scalar power);


// * * * * * * * * * * * * * * * Global Operators  * * * * * * * * * * * * * //

Istream& operator>>(Istream& is, quaternion& q);
Ostream& operator<<(Ostream& os, const quaternion& q);

inline bool operator==(const quaternion& q1, const quaternion& q2);
inline bool operator!=(const quaternion& q1, const quaternion& q2);
inline quaternion operator+(const quaternion& q1, const quaternion& q2);
inline quaternion operator-(const quaternion& q);
inline quaternion operator-(const quaternion& q1, const quaternion& q2);
inline scalar operator&(const quaternion& q1, const quaternion& q2);
inline quaternion operator*(const quaternion& q1, const quaternion& q2);
inline quaternion operator/(const quaternion& q1, const quaternion& q2);
inline quaternion operator*(const scalar s, const quaternion& q);
inline quaternion operator*(const quaternion& q, const scalar s);
inline quaternion operator/(const quaternion& q, const scalar s);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "primitives/quaternion/quaternionI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
