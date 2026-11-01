// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Darkstalkers as one of the games made from Night Warriors' program, whose sources it builds (games/nwarr/src;
// places.h there): where the places those sources name are in this game.
#pragma once

#include <array>
#include <cstdint>

namespace nwarr::family {

struct Place {
    uint32_t nwarr, here;
};
inline constexpr Place kPlaceList[] = {
    // places: begin (tools/places.py; a line ending "by hand" is kept)
    {0x0064D6, 0x005968}, // an instruction, 4 on alike
    {0x006502, 0x0059C6}, // an instruction, 4 on alike
    {0x00692E, 0x005F96}, // an instruction, 1 on alike
    {0x006B70, 0x0061C8}, // a place the code names so (1/1)
    {0x006BAC, 0x0061FC}, // an instruction, 2 on alike
    {0x008798, 0x00796C}, // an instruction, 10 on alike
    {0x01FCDC, 0x02AB92}, // an instruction, 1 on alike
    {0x02A220, 0x033FF4}, // an instruction, 15 on alike
    {0x02C738, 0x036304}, // an instruction, 3 on alike
    {0x02ED20, 0x037C78}, // an instruction, 3 on alike
    {0x0302C6, 0x038A4E}, // an instruction, 8 on alike
    {0x0303BC, 0x038B42}, // an instruction, 1 on alike
    {0x030626, 0x038D52}, // an instruction, 3 on alike
    {0x030636, 0x038D62}, // an instruction, 7 on alike
    {0x03096E, 0x038FD2}, // an instruction, 2 on alike
    {0x03787A, 0x03C442}, // an instruction, 2 on alike
    {0x03AD7C, 0x03F700}, // an instruction, 2 on alike
    {0x03BD18, 0x040346}, // an instruction, 8 on alike
    {0x0421A0, 0x0438CE}, // an instruction, 5 on alike
    {0x043C2C, 0x0458B6}, // an instruction, 3 on alike
    {0x04A2EC, 0x04AC56}, // an instruction, 6 on alike
    {0x04A4D4, 0x04AE3E}, // an instruction, 9 on alike
    {0x04A5FE, 0x04AEE2}, // an instruction, 5 on alike
    {0x04C8E8, 0x04C690}, // an instruction, 2 on alike
    {0x068CFC, 0x05664E}, // an instruction, 6 on alike
    {0x0DA878, 0x0CE502}, // the same bytes, once only
    {0x0FF6D6, 0x0EDB06}, // the same entry of a jump table, 4 on alike
    {0x0FF6F6, 0x0EDB26}, // an instruction, 16 on alike
    {0x0FF780, 0x0EDBB0}, // an instruction, 10 on alike
    {0x0FF7BA, 0x0EDBEA}, // an instruction, 10 on alike
    {0x35DD0E, 0x30F3DC}, // a place the code names so (2/2)
    {0xFF0AA0, 0xFF0A9E}, // a variable the code names so (4/4)
    {0xFF8000, 0xFF8000}, // a variable the code names so (28/28)
    {0xFF8004, 0xFF8004}, // a variable the code names so (36/36)
    {0xFF8008, 0xFF8008}, // a variable the code names so (33/33)
    {0xFF800C, 0xFF800C}, // a variable the code names so (51/53)
    {0xFF8026, 0xFF8026}, // a variable the code names so (6/6)
    {0xFF808E, 0xFF808E}, // a variable the code names so (11/11)
    {0xFF8092, 0xFF8092}, // a variable the code names so (11/11)
    {0xFF81B2, 0xFF81D0}, // a variable the code names so (2/2)
    {0xFF81EB, 0xFF81EB}, // a variable the code names so (3/3)
    {0xFF81F0, 0xFF81F0}, // a variable the code names so (2/2)
    {0xFF81F1, 0xFF81F1}, // a variable the code names so (2/2)
    {0xFF81F9, 0xFF81F9}, // a variable the code names so (13/13)
    {0xFF8203, 0xFF81F3}, // a variable the code names so (1/1)
    {0xFF8212, 0xFF8212}, // a variable the code names so (2/2)
    {0xFF8233, 0xFF8233}, // a variable the code names so (1/1)
    {0xFF8237, 0xFF8237}, // a variable between two that moved alike
    {0xFF8263, 0xFF8263}, // a variable the code names so (2/2)
    {0xFF8308, 0xFF8308}, // a variable the code names so (7/7)
    {0xFF8388, 0xFF8388}, // a variable the code names so (47/48)
    {0xFF8D88, 0xFF9388}, // a variable the code names so (30/30)
    {0xFF8D98, 0xFF9398}, // a variable the code names so (3/3)
    {0xFF8D9A, 0xFF939A}, // a variable the code names so (3/3)
    {0xFF8E09, 0xFF9409}, // a variable the code names so (3/3)
    {0xFF8E14, 0xFF9414}, // a variable the code names so (1/1)
    {0xFF8E15, 0xFF9415}, // a variable the code names so (4/4)
    {0xFF8F08, 0xFF9508}, // a variable the code names so (3/3)
    {0xFF8F18, 0xFF9518}, // a variable the code names so (35/35)
    {0xFF8F1C, 0xFF951C}, // a variable the code names so (7/7)
    {0xFFA86E, 0xFFAA2E}, // a variable the code names so (13/13)
    {0xFFA92E, 0xFFAAEE}, // a variable the code names so (8/8)
    {0xFFACEE, 0xFFABAE}, // a variable the code names so (10/10)
    {0xFFADAE, 0xFFAC6E}, // a variable the code names so (8/8)
    // places: end
};
inline constexpr auto kPlaces = std::to_array(kPlaceList);

} // namespace nwarr::family
