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

\*---------------------------------------------------------------------------*/

#include "rampedSixDoFMotion.H"
#include "addToRunTimeSelectionTable.H"
#include "unitConversion.H"
#include "mathematicalConstants.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solidBodyMotionFunctions
{

    defineTypeNameAndDebug(rampedSixDoFMotion, 0);
    addToRunTimeSelectionTable
    (
        solidBodyMotionFunction,
        rampedSixDoFMotion,
        dictionary
    );

}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solidBodyMotionFunctions::rampedSixDoFMotion::rampedSixDoFMotion
(
    const dictionary& SBMFCoeffs,
    const Time& runTime
)
:
    solidBodyMotionFunction(SBMFCoeffs, runTime)
{
    read(SBMFCoeffs);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::septernion
Foam::solidBodyMotionFunctions::rampedSixDoFMotion::transformation() const
{
    const scalar t = time_.value();

    // Optional linear ramp
    scalar ramp = 1.0;
    if (ramp_ && t < rampTime_)
    {
        ramp = t/rampTime_;
    }

    // Surge, sway and heave
    const vector translation
    (
        amplitudeTranslation_.x()*ramp
       *sin(constant::mathematical::twoPi*frequencyTranslation_.x()*t),

        amplitudeTranslation_.y()*ramp
       *sin(constant::mathematical::twoPi*frequencyTranslation_.y()*t),

        amplitudeTranslation_.z()*ramp
       *sin(constant::mathematical::twoPi*frequencyTranslation_.z()*t)
    );

    // Roll, pitch and yaw, converted from degrees to radians
    const vector rotationDeg
    (
        amplitudeRotation_.x()*ramp
       *sin(constant::mathematical::twoPi*frequencyRotation_.x()*t),

        amplitudeRotation_.y()*ramp
       *sin(constant::mathematical::twoPi*frequencyRotation_.y()*t),

        amplitudeRotation_.z()*ramp
       *sin(constant::mathematical::twoPi*frequencyRotation_.z()*t)
    );

    const quaternion R(quaternion::XYZ, rotationDeg*degToRad());

    return septernion
    (
        septernion(-origin_ + -translation)*R*septernion(origin_)
    );
}


bool
Foam::solidBodyMotionFunctions::rampedSixDoFMotion::read
(
    const dictionary& SBMFCoeffs
)
{
    solidBodyMotionFunction::read(SBMFCoeffs);

    SBMFCoeffs_.readEntry("origin", origin_);
    SBMFCoeffs_.readEntry("amplitudeTranslation", amplitudeTranslation_);
    SBMFCoeffs_.readEntry("amplitudeRotation", amplitudeRotation_);
    SBMFCoeffs_.readEntry("frequencyTranslation", frequencyTranslation_);
    SBMFCoeffs_.readEntry("frequencyRotation", frequencyRotation_);
    SBMFCoeffs_.readEntry("ramp", ramp_);
    SBMFCoeffs_.readIfPresent("rampTime", rampTime_, keyType::LITERAL);

    return true;
}


// ************************************************************************* //
