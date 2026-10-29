// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
package io.github.tmcleroy.cps2recomp;

import android.content.Intent;
import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;

import org.libsdl.app.SDLActivity;

// A game: its library (lib<id>.so), loaded in a process of its own, whose main() SDL calls on its own thread with the
// arguments the launcher gave (LauncherActivity.startGame). What main() returns is the activity's result, which the
// launcher reads as a program's exit code (the Multi game page's hand-off to the next game among them). The process
// ends with the game: the library keeps the game's state in globals, which a second start in it would find used.
public class GameActivity extends SDLActivity {
    static final String EXTRA_LIBRARY = "library";   // the game's id: lib<id>.so
    static final String EXTRA_ARGUMENTS = "arguments";
    static final String EXTRA_FOLDER = "folder";     // the player's folder for the game: its current folder
    static final String EXTRA_LOG = "log";           // where its output goes (logs/game.log)

    // One activity for each process (AndroidManifest.xml)
    public static class Slot0 extends GameActivity {}
    public static class Slot1 extends GameActivity {}
    public static class Slot2 extends GameActivity {}
    public static class Slot3 extends GameActivity {}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // (port/android.cpp reads these before anything else)
        Intent intent = getIntent();
        try {
            Os.setenv("RECOMP_FOLDER", text(intent.getStringExtra(EXTRA_FOLDER)), true);
            Os.setenv("RECOMP_LOG", text(intent.getStringExtra(EXTRA_LOG)), true);
        } catch (ErrnoException e) {
            Log.w("cps2recomp", "setenv: " + e);
        }
        super.onCreate(savedInstanceState);
    }

    private static String text(String s) {
        return s == null ? "" : s;
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {"SDL3", text(getIntent().getStringExtra(EXTRA_LIBRARY))};
    }

    @Override
    protected String[] getArguments() {
        String[] arguments = getIntent().getStringArrayExtra(EXTRA_ARGUMENTS);
        return arguments == null ? new String[0] : arguments;
    }

    // An HTTP request, from curl's arguments (recomp-common/launcher/src/run_program.cpp): Curl.java
    public String curl(String[] arguments) {
        return Curl.run(arguments);
    }

    @Override
    protected void main() {
        String library = getMainSharedObject();
        int code = SDLActivity.nativeRunMain(library, getMainFunction(), getArguments());
        Log.v("cps2recomp", getIntent().getStringExtra(EXTRA_LIBRARY) + " ended with " + code);
        // (RESULT_FIRST_USER and up: a process that ended some other way, a crash, gives RESULT_CANCELED)
        setResult(RESULT_FIRST_USER + code);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (isFinishing()) {
            System.exit(0);
        }
    }
}
