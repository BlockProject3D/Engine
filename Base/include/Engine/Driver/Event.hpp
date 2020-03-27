// Copyright (c) 2020, BlockProject
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of BlockProject nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <Framework/Types.hpp>
#include "Engine/Input/EKey.hpp"

namespace bp3d
{
    namespace driver
    {
        enum class BP3D_API EEventType
        {
            KEY,
            TEXT,
            AXIS,
            VR,
            DISPLAY_CLOSE,
            DISPLAY_RESIZE
        };

        enum class BP3D_API EAxisType
        {
            MOUSE_XY,
            JOYSTICK_LEFT_X,
            JOYSTICK_LEFT_Y,
            JOYSTICK_RIGHT_X,
            JOYSTICK_RIGHT_Y,
            JOYSTICK_TRIGGER_LEFT_X,
            JOYSTICK_TRIGGER_RIGHT_X,
            JOYSTICK_MAX_X
        };

        struct BP3D_API EventText
        {
            EEventType Type;
            char Text[32];
        };

        struct BP3D_API EventVR
        {
            EEventType Type;
            bpf::fint JoyconId;
            float JoyconX;
            float JoyconY;
            float JoyconZ;
        };

        struct BP3D_API EventDisplayResize
        {
            EEventType Type;
            bpf::fsize Width;
            bpf::fsize Height;
        };

        struct BP3D_API EventAxis
        {
            EEventType Type;
            EAxisType AxisType;
            bpf::fint X;
            bpf::fint Y;
        };

        struct BP3D_API EventKey
        {
            EEventType Type;
            input::EKey Key;
            bool Pressed;
        };

        union BP3D_API Event
        {
            EEventType Type;
            EventText Text;
            EventKey Key;
            EventAxis Axis;
            EventVR VR;
            EventDisplayResize DisplayResize;
        };
    }
}