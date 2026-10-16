// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound program (driver
// 1.30) keeps a table of its songs at 0x9006, four bytes a song; a song whose data starts with a 0 is music.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x6D8;
    // Found by playing: each stage, the arcade ladder to the end with every character (player 1's character byte
    // held), a lost game and attract mode (CPS2RECOMP_SOUNDTRACK_LOG), and --music-test for the ones the board plays
    // once (`true`: quiet before 90 s)
    TheGame().songs = {
        // Each stage's own (modes.h, kStageTune)
        {0x09, "feast_of_the_damned", "Feast of the Damned"},
        {0x0A, "concrete_cave", "Concrete Cave"},
        {0x0B, "tower_of_arrogance", "Tower of Arrogance"},
        {0x0C, "red_thirst", "Red Thirst"},
        {0x0D, "deserted_chateau", "Deserted Chateau"},
        {0x0E, "abaraya", "Abaraya"},
        {0x0F, "vanity_paradise", "Vanity Paradise"},
        {0x10, "war_agony", "War Agony"},
        {0x11, "forever_torment", "Forever Torment"},
        {0x12, "green_scream", "Green Scream"},
        {0x13, "iron_horse_iron_terror", "Iron Horse, Iron Terror"},
        {0x14, "fetus_of_god", "Fetus of God"},
        // Around the fights
        {0x55, "qsound", "The QSound logo", true},
        {0x57, "capcom", "The Capcom logo", true},
        {0x01, "opening", "The opening", true},
        {0x02, "character_select", "Character select"},
        {0x03, "next", "Between a win and the next versus screen"},
        {0x05, "versus", "Versus screen", true},
        {0x47, "rival", "Before the last fight (the rival)"},
        {0xE2, "jedah_appears", "Jedah appears", true},
        {0x06, "continue", "Continue?"},
        {0x07, "game_over", "Game over", true},
        {0x08, "ranking", "Score ranking", true},
        {0x45, "staff_roll", "Staff roll"},
        // The winner's, on the screen after a win
        {0x15, "bulleta_win", "Bulleta wins"},
        {0x16, "demitri_win", "Demitri wins"},
        {0x17, "gallon_win", "Gallon wins"},
        {0x18, "victor_win", "Victor wins"},
        {0x19, "zabel_win", "Zabel wins"},
        {0x1A, "morrigan_win", "Morrigan wins"},
        {0x1B, "anakaris_win", "Anakaris wins"},
        {0x1C, "felicia_win", "Felicia wins"},
        {0x1D, "bishamon_win", "Bishamon wins"},
        {0x1E, "aulbath_win", "Aulbath wins"},
        {0x1F, "sasquatch_win", "Sasquatch wins"},
        {0x21, "qbee_win", "Q-Bee wins"},
        {0x22, "leilei_win", "Lei-Lei wins"},
        {0x23, "lilith_win", "Lilith wins"},
        {0x24, "jedah_win", "Jedah wins"},
        // The endings (Q-Bee's and Lei-Lei's by the order of the others: not seen yet)
        {0x25, "bulleta_ending", "Bulleta's ending"},
        {0x26, "demitri_ending", "Demitri's ending", true},
        {0x27, "gallon_ending", "Gallon's ending", true},
        {0x28, "victor_ending", "Victor's ending"},
        {0x29, "zabel_ending", "Zabel's ending"},
        {0x2A, "morrigan_ending", "Morrigan's ending"},
        {0x2B, "anakaris_ending", "Anakaris's ending"},
        {0x2C, "felicia_ending", "Felicia's ending"},
        {0x2D, "bishamon_ending", "Bishamon's ending"},
        {0x2E, "aulbath_ending", "Aulbath's ending"},
        {0x2F, "sasquatch_ending", "Sasquatch's ending"},
        {0x31, "qbee_ending", "Q-Bee's ending"},
        {0x32, "leilei_ending", "Lei-Lei's ending"},
        {0x33, "lilith_ending", "Lilith's ending"},
        {0x34, "jedah_ending", "Jedah's ending"},
        // Music the game has whose place is not known yet
        {0x04, "music_04", "Where it plays is not known yet", true},
        {0x35, "music_35", "Where it plays is not known yet"},
        {0x36, "music_36", "Where it plays is not known yet"},
        {0x37, "music_37", "Where it plays is not known yet"},
        {0x38, "music_38", "Where it plays is not known yet"},
        {0x39, "music_39", "Where it plays is not known yet"},
        {0x3A, "music_3a", "Where it plays is not known yet"},
        {0x3B, "music_3b", "Where it plays is not known yet"},
        {0x3C, "music_3c", "Where it plays is not known yet"},
        {0x3D, "music_3d", "Where it plays is not known yet"},
        {0x3E, "music_3e", "Where it plays is not known yet"},
        {0x3F, "music_3f", "Where it plays is not known yet"},
        {0x42, "music_42", "Where it plays is not known yet"},
        {0x43, "music_43", "Where it plays is not known yet"},
        {0x44, "music_44", "Where it plays is not known yet"},
        {0x46, "music_46", "Where it plays is not known yet"},
        {0x4E, "music_4e", "Where it plays is not known yet"},
        {0x4F, "music_4f", "Where it plays is not known yet"},
        {0x50, "music_50", "Where it plays is not known yet"},
        {0x51, "music_51", "Where it plays is not known yet"},
        {0x52, "music_52", "Where it plays is not known yet"},
        {0x53, "music_53", "Where it plays is not known yet"},
        {0x54, "music_54", "Where it plays is not known yet", true},
        {0x478, "music_478", "Where it plays is not known yet", true},
    };
}

} // namespace cps2recomp
