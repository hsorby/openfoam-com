/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2016-2023 OpenCFD Ltd.
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
    Foam::wordRes

Description
    A List of wordRe with additional matching capabilities.

SourceFiles
    wordResI.H
    wordRes.C

\*---------------------------------------------------------------------------*/

#ifndef Foam_wordRes_H
#define Foam_wordRes_H

#include "primitives/strings/wordRe/wordRe.H"
#include "containers/Lists/List/List.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// List types
typedef List<wordRe> wordReList;    //!< List of wordRe (word or regex)
typedef UList<wordRe> wordReUList;  //!< UList of wordRe (word or regex)


/*---------------------------------------------------------------------------*\
                           Class wordRes Declaration
\*---------------------------------------------------------------------------*/

class wordRes
:
    public List<wordRe>
{
    // Private Methods

        //- Smart match as literal or regex, stopping on the first match.
        //  \return index of first match, -1 if not found
        inline static label first_match
        (
            const UList<wordRe>& selectors,
            const std::string& text,
            const bool literal=false
        );

        //- Smart match across entire list, returning the best match type.
        //  Stops on the first literal match, or continues to examine
        //  if a regex match occurs.
        //  \return wordRe::LITERAL, wordRe::REGEX on match and
        //      wordRe::UNKNOWN otherwise.
        inline static wordRe::compOption found_matched
        (
            const UList<wordRe>& selectors,
            const std::string& text
        );


public:

    // Static Data / Methods

        //- Return a null wordRes - a reference to the NullObject
        inline static const wordRes& null();

        //- Return a wordRes with duplicate entries filtered out.
        //  No distinction made between literals and regular expressions.
        static wordRes uniq(const UList<wordRe>& input);


    // Constructors

        //- Inherit constructors from List of wordRe
        using List<wordRe>::List;


    //- Destructor
    ~wordRes() = default;


    // Member Functions

        //- Filter out duplicate entries (inplace).
        //  No distinction made between literals and regular expressions.
        void uniq();

        //- Smart match as literal or regex, stopping on the first match.
        //
        //  \param literal Force literal match only.
        //  \return True if text matches ANY of the entries.
        inline bool match(const std::string& text, bool literal=false) const;

        //- Smart match in the list of matchers, returning the match type.
        //  It stops if there is a literal match, or continues to examine
        //  other regexs.
        //  \return LITERAL if a lteral match was found,
        //      REGEX if any regex match was found,
        //      UNKNOWN otherwise.
        inline wordRe::compOption matched(const std::string& text) const;

        //- Return list indices for all matches.
        //
        //  \param input  A list of string inputs to match against
        //  \param invert invert the matching logic
        //  \return indices of the matches in the input list
        template<class StringType>
        inline labelList matching
        (
            const UList<StringType>& input,
            const bool invert=false
        ) const;


    // Member Operators

        //- Identical to match(), for use as a predicate.
        inline bool operator()(const std::string& text) const;


    // Functors

        //- Functor wrapper of a list of wordRe for matching
        struct matcher
        {
            //- Construct with 'allow' matcher
            inline matcher(const UList<wordRe>& allow);

            //- Nothing defined
            inline bool empty() const noexcept;

            //- True if text matches ANY of the entries.
            //  Allows use as a predicate.
            inline bool operator()(const std::string& text) const;

        private:
            const UList<wordRe>& allow_;
        };


        //- Functor wrapper of allow/deny lists of wordRe for filtering
        //
        //  An empty 'allow' accepts everything not in 'deny'.
        //  A literal 'allow' match has higher priority than any 'deny'.
        //  A regex 'allow' match has lower priority than any 'deny'.
        //
        //  Example (when applied to a list of words),
        //  \verbatim
        //     input:  ( abc apple test other val val1 val2 wall wall1 wall2 )
        //     allow:  ( abc def "t.*" other val val1 "wall.*" )
        //     deny:   ( "[ab].*" "t.*" other "val[0-9]" wall )
        //
        //     result:  (abc other val val1 wall1 wall2)
        //  \endverbatim
        struct filter
        {
            //- Construct with 'allow' and 'deny' matchers
            inline filter
            (
                const UList<wordRe>& allow,
                const UList<wordRe>& deny
            );

            //- Nothing defined
            inline bool empty() const noexcept;

            //- True if matched but not blocked
            inline bool operator()(const std::string& text) const;

        private:
            const UList<wordRe>& allow_;
            const UList<wordRe>& deny_;
        };
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "primitives/strings/wordRes/wordResI.H"

#endif

// ************************************************************************* //
