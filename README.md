# rampedSixDoFMotion 自定义运动函数说明

`rampedSixDoFMotion` 是自定义的 OpenFOAM `solidBodyMotionFunction`，用于给液舱
施加六自由度简谐运动，并支持可选线性缓冲。运动公式对应文献
《简谐激励下带制荡隔板的液舱晃荡特性》式（5）的推广形式。

## 1. 运动公式

三个平动自由度（surge、sway、heave）：

```text
translation_i = amplitudeTranslation_i * ramp(t) * sin(2*pi*frequencyTranslation_i*t)
```

三个转动自由度（roll、pitch、yaw），幅值单位为度：

```text
rotation_i = amplitudeRotation_i * ramp(t) * sin(2*pi*frequencyRotation_i*t)
```

线性缓冲函数：

```text
ramp = on:
    ramp(t) = t/rampTime   for t < rampTime
    ramp(t) = 1            for t >= rampTime

ramp = off:
    ramp(t) = 1            for all t
```

转动使用 XYZ 欧拉角顺序，即 roll 绕 x 轴、pitch 绕 y 轴、yaw 绕 z 轴，转动中心
为 `origin`。

## 2. 文件结构

```text
customMotionFunction/
├── rampedSixDoFMotion.H
├── rampedSixDoFMotion.C
├── Make/
│   ├── files
│   └── options
├── Allwmake
└── README.md
```

## 3. 编译

在 WSL 中进入 `customMotionFunction` 目录并执行：

```bash
source /usr/lib/openfoam/openfoam2412/etc/bashrc
wmake
```

或者直接执行：

```bash
./Allwmake
```

编译成功后生成：

```text
$FOAM_USER_LIBBIN/librampedSixDoFMotion.so
```

### 3.1 编译报错处理

如果报“找不到 `solidBodyMotionFunction.H`”，先确认编译前已经 source OpenFOAM
环境。本函数的 `Make/options` 同时包含 `LIB_SRC` 和 `FOAM_SRC` 两条头文件路径，
可兼容不同 OpenFOAM 环境。

仍然找不到时检查：

```text
$FOAM_SRC/meshTools/lnInclude/solidBodyMotionFunction.H
```

如果报错为 `invalid new-expression` / 纯虚函数，说明自定义类缺少基类 `clone()`，
源码中已实现该函数。

## 4. dynamicMeshDict 配置方法

在算例 `constant/dynamicMeshDict` 中启用该函数：

```text
dynamicFvMesh       dynamicMotionSolverFvMesh;

motionSolverLibs    (rampedSixDoFMotion);

motionSolver        solidBody;

solidBodyMotionFunction rampedSixDoFMotion;

rampedSixDoFMotionCoeffs
{
    origin              (0.5 0.25 0.51);

    amplitudeTranslation (0 0 0);      // surge, sway, heave [m]
    amplitudeRotation    (0 2 0);      // roll, pitch, yaw [deg]
    frequencyTranslation (0 0 0);      // Hz
    frequencyRotation    (0 0.66 0);   // Hz

    ramp                on;            // on/off linear ramp
    rampTime            10;            // s, only used when ramp = on
}
```

### 4.1 参数说明

| 参数 | 含义 | 单位 | 示例 |
| --- | --- | --- | --- |
| `origin` | 转动/平动参考中心 | m | `(0.5 0.25 0.51)` |
| `amplitudeTranslation` | surge/sway/heave 幅值 | m | `(0 0 0)` |
| `amplitudeRotation` | roll/pitch/yaw 幅值 | deg | `(0 2 0)` |
| `frequencyTranslation` | 平动频率 | Hz | `(0 0 0)` |
| `frequencyRotation` | 转动频率 | Hz | `(0 0.66 0)` |
| `ramp` | 是否启用线性缓冲 | on/off | `on` |
| `rampTime` | 缓冲时间 | s | `10` |

### 4.2 常用配置

只做纵摇，带 10 s 线性缓冲（文献式（5））：

```text
rampedSixDoFMotionCoeffs
{
    origin              (0.5 0.25 0.51);
    amplitudeTranslation (0 0 0);
    amplitudeRotation    (0 2 0);
    frequencyTranslation (0 0 0);
    frequencyRotation    (0 0.66 0);
    ramp                on;
    rampTime            10;
}
```

只做纵摇，不加缓冲：

```text
rampedSixDoFMotionCoeffs
{
    origin              (0.5 0.25 0.51);
    amplitudeTranslation (0 0 0);
    amplitudeRotation    (0 2 0);
    frequencyTranslation (0 0 0);
    frequencyRotation    (0 0.66 0);
    ramp                off;
    rampTime            10;
}
```

同时施加纵荡、垂荡和纵摇：

```text
rampedSixDoFMotionCoeffs
{
    origin              (0.5 0.25 0.51);
    amplitudeTranslation (0.01 0 0.005);  // surge 1 cm, heave 0.5 cm
    amplitudeRotation    (0 2 0);          // pitch 2 deg
    frequencyTranslation (0.66 0 0.66);
    frequencyRotation    (0 0.66 0);
    ramp                on;
    rampTime            10;
}
```

## 5. 完整源码

### 5.1 rampedSixDoFMotion.H

```cpp
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
```

### 5.2 rampedSixDoFMotion.C

```cpp
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
```

### 5.3 Make/files

```text
rampedSixDoFMotion.C

LIB = $(FOAM_USER_LIBBIN)/librampedSixDoFMotion
```

### 5.4 Make/options

```text
EXE_INC = \
    -I$(LIB_SRC)/meshTools/lnInclude \
    -I$(FOAM_SRC)/meshTools/lnInclude

LIB_LIBS = \
    -lmeshTools
```

### 5.5 Allwmake

```bash
#!/bin/sh
cd "${0%/*}" || exit

wmake
```

## 6. 运行算例

编译完成后，回到算例目录执行：

```bash
./Allclean
./Allrun
```

`Allrun` 会依次执行 `blockMesh`、`topoSet`、`subsetMesh`、`setFields` 和
`interFoam`，并自动加载 `librampedSixDoFMotion.so`。