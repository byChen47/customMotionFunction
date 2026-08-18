/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2026
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
    Foam::solidBodyMotionFunctions::rampedSixDoFMotion

Description
    Six-DOF harmonic motion with an optional linear ramp.

    Each translation component (surge, sway, heave) and each rotation
    component (roll, pitch, yaw) has its own amplitude and frequency:

        translation_i = amplitudeTranslation_i * ramp(t)
                       *sin(2*pi*frequencyTranslation_i*t)

        rotation_i    = amplitudeRotation_i * ramp(t)
                       *sin(2*pi*frequencyRotation_i*t)

    Rotation amplitudes are in degrees. The ramp function is:

        ramp(t) = t/rampTime          for t < rampTime, if ramp = on
        ramp(t) = 1                   otherwise

    With ramp = off the motion starts at full amplitude from t = 0.
    The rotations are applied about "origin" using XYZ Euler angles,
    i.e. roll about x, pitch about y and yaw about z.

SourceFiles
    rampedSixDoFMotion.C

\*---------------------------------------------------------------------------*/

#ifndef rampedSixDoFMotion_H
#define rampedSixDoFMotion_H

#include "solidBodyMotionFunction.H"
#include "primitiveFields.H"
#include "point.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solidBodyMotionFunctions
{

/*---------------------------------------------------------------------------*\
                    Class rampedSixDoFMotion Declaration
\*---------------------------------------------------------------------------*/

class rampedSixDoFMotion
:
    public solidBodyMotionFunction
{
    // Private Data

        //- Rotation/translation reference centre
        point origin_;

        //- Surge, sway and heave amplitudes in metres
        vector amplitudeTranslation_;

        //- Roll, pitch and yaw amplitudes in degrees
        vector amplitudeRotation_;

        //- Surge, sway and heave frequencies in Hz
        vector frequencyTranslation_;

        //- Roll, pitch and yaw frequencies in Hz
        vector frequencyRotation_;

        //- Switch the linear ramp on/off
        bool ramp_;

        //- Ramp time in seconds
        scalar rampTime_;


public:

    //- Runtime type information
    TypeName("rampedSixDoFMotion");


    // Constructors

        //- Construct from dictionary
        rampedSixDoFMotion
        (
            const dictionary& SBMFCoeffs,
            const Time& runTime
        );

        //- Construct and return a clone
        virtual autoPtr<solidBodyMotionFunction> clone() const
        {
            return autoPtr<solidBodyMotionFunction>
            (
                new rampedSixDoFMotion(SBMFCoeffs_, time_)
            );
        }

        //- No copy construct
        rampedSixDoFMotion(const rampedSixDoFMotion&) = delete;

        //- No copy assignment
        void operator=(const rampedSixDoFMotion&) = delete;


    //- Destructor
    virtual ~rampedSixDoFMotion() = default;


    // Member Functions

        //- Return the solid-body motion transformation septernion
        virtual septernion transformation() const;

        //- Update properties from given dictionary
        virtual bool read(const dictionary& SBMFCoeffs);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solidBodyMotionFunctions
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
