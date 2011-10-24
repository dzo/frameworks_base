/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.os;

import android.app.ActivityManagerNative;
import android.app.ApplicationErrorReport;
import android.os.Build;
import android.os.Debug;
import android.os.IBinder;
import android.os.Process;
import android.os.SystemProperties;
import android.util.Config;
import android.util.Log;
import android.util.Slog;

import com.android.internal.logging.AndroidConfig;

import dalvik.system.VMRuntime;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.LogManager;
import java.util.TimeZone;

import org.apache.harmony.luni.internal.util.TimezoneGetter;

/**
 * Main entry point for runtime initialization.  Not for
 * public consumption.
 * @hide
 */
public class RuntimeInit {
    private final static String TAG = "AndroidRuntime";

    /** true if commonInit() has been called */
    private static boolean initialized;

    private static IBinder mApplicationObject;

    private static volatile boolean mCrashing = false;

    /**
     * Use this to log a message when a thread exits due to an uncaught
     * exception.  The framework catches these for the main threads, so
     * this should only matter for threads created by applications.
     */
    private static class UncaughtHandler implements Thread.UncaughtExceptionHandler {
        public void uncaughtException(Thread t, Throwable e) {
            try {
                // Don't re-enter -- avoid infinite loops if crash-reporting crashes.
                if (mCrashing) return;
                mCrashing = true;

                if (mApplicationObject == null) {
                    Slog.e(TAG, "*** FATAL EXCEPTION IN SYSTEM PROCESS: " + t.getName(), e);
                } else {
                    Slog.e(TAG, "FATAL EXCEPTION: " + t.getName(), e);
                }

                // Bring up crash dialog, wait for it to be dismissed
                ActivityManagerNative.getDefault().handleApplicationCrash(
                        mApplicationObject, new ApplicationErrorReport.CrashInfo(e));
            } catch (Throwable t2) {
                try {
                    Slog.e(TAG, "Error reporting crash", t2);
                } catch (Throwable t3) {
                    // Even Slog.e() fails!  Oh well.
                }
            } finally {
                // Try everything to make sure this process goes away.
                Process.killProcess(Process.myPid());
                System.exit(10);
            }
        }
    }

    private static final void commonInit() {
        if (Config.LOGV) Slog.d(TAG, "Entered RuntimeInit!");

        /* set default handler; this applies to all threads in the VM */
        Thread.setDefaultUncaughtExceptionHandler(new UncaughtHandler());

        int hasQwerty = getQwertyKeyboard();

        if (Config.LOGV) Slog.d(TAG, ">>>>> qwerty keyboard = " + hasQwerty);
        if (hasQwerty == 1) {
            System.setProperty("qwerty", "1");
        }

        /*
         * Install a TimezoneGetter subclass for ZoneInfo.db
         */
        TimezoneGetter.setInstance(new TimezoneGetter() {
            @Override
            public String getId() {
                return SystemProperties.get("persist.sys.timezone");
            }
        });
        TimeZone.setDefault(null);

        /*
         * Sets handler for java.util.logging to use Android log facilities.
         * The odd "new instance-and-then-throw-away" is a mirror of how
         * the "java.util.logging.config.class" system property works. We
         * can't use the system property here since the logger has almost
         * certainly already been initialized.
         */
        LogManager.getLogManager().reset();
        new AndroidConfig();

        /*
         * Sets the default HTTP User-Agent used by HttpURLConnection.
         */
        String userAgent = getDefaultUserAgent();
        System.setProperty("http.agent", userAgent);

        /*
         * If we're running in an emulator launched with "-trace", put the
         * VM into emulator trace profiling mode so that the user can hit
         * F9/F10 at any time to capture traces.  This has performance
         * consequences, so it's not something you want to do always.
         */
        String trace = SystemProperties.get("ro.kernel.android.tracing");
        if (trace.equals("1")) {
            Slog.i(TAG, "NOTE: emulator trace profiling enabled");
            Debug.enableEmulatorTraceOutput();
        }

        initialized = true;
    }

    /**
     * Returns an HTTP user agent of the form
     * "Dalvik/1.1.0 (Linux; U; Android Eclair Build/MASTER)".
     */
    private static String getDefaultUserAgent() {
        StringBuilder result = new StringBuilder(64);
        result.append("Dalvik/");
        result.append(System.getProperty("java.vm.version")); // such as 1.1.0
        result.append(" (Linux; U; Android ");

        String version = Build.VERSION.RELEASE; // "1.0" or "3.4b5"
        result.append(version.length() > 0 ? version : "1.0");

        // add the model for the release build
        if ("REL".equals(Build.VERSION.CODENAME)) {
            String model = Build.MODEL;
            if (model.length() > 0) {
                result.append("; ");
                result.append(model);
            }
        }
        String id = Build.DISPLAY; // "MASTER" or "M4-rc20"
        if (id.length() > 0) {
            result.append(" Build/");
            result.append(id);
        }
        result.append(")");
        return result.toString();
    }

    /**
     * Invokes a static "main(argv[]) method on class "className".
     * Converts various failing exceptions into RuntimeExceptions, with
     * the assumption that they will then cause the VM instance to exit.
     *
     * @param className Fully-qualified class name
     * @param argv Argument vector for main()
     */
    private static void invokeStaticMain(String className, String[] argv)
            throws ZygoteInit.MethodAndArgsCaller {

        // We want to be fairly aggressive about heap utilization, to avoid
        // holding on to a lot of memory that isn't needed.
        VMRuntime.getRuntime().setTargetHeapUtilization(0.75f);

        Class<?> cl;

        try {
            cl = Class.forName(className);
        } catch (ClassNotFoundException ex) {
            throw new RuntimeException(
                    "Missing class when invoking static main " + className,
                    ex);
        }

        Method m;
        try {
            m = cl.getMethod("main", new Class[] { String[].class });
        } catch (NoSuchMethodException ex) {
            throw new RuntimeException(
                    "Missing static main on " + className, ex);
        } catch (SecurityException ex) {
            throw new RuntimeException(
                    "Problem getting static main on " + className, ex);
        }

        int modifiers = m.getModifiers();
        if (! (Modifier.isStatic(modifiers) && Modifier.isPublic(modifiers))) {
            throw new RuntimeException(
                    "Main method is not public and static on " + className);
        }

        /*
         * This throw gets caught in ZygoteInit.main(), which responds
         * by invoking the exception's run() method. This arrangement
         * clears up all the stack frames that were required in setting
         * up the process.
         */
        throw new ZygoteInit.MethodAndArgsCaller(m, argv);
    }

    public static final void main(String[] argv) {
        commonInit();

        /*
         * Now that we're running in interpreted code, call back into native code
         * to run the system.
         */
        finishInit();

        if (Config.LOGV) Slog.d(TAG, "Leaving RuntimeInit!");
    }

    public static final native void finishInit();

    /**
     * The main function called when started through the zygote process. This
     * could be unified with main(), if the native code in finishInit()
     * were rationalized with Zygote startup.<p>
     *
     * Current recognized args:
     * <ul>
     *   <li> --nice-name=<i>nice name to appear in ps</i>
     *   <li> <code> [--] &lt;start class name&gt;  &lt;args&gt;
     * </ul>
     *
     * @param argv arg strings
     */
    public static final void zygoteInit(String[] argv)
            throws ZygoteInit.MethodAndArgsCaller {
        // TODO: Doing this here works, but it seems kind of arbitrary. Find
        // a better place. The goal is to set it up for applications, but not
        // tools like am.
        System.setOut(new AndroidPrintStream(Log.INFO, "System.out"));
        System.setErr(new AndroidPrintStream(Log.WARN, "System.err"));

        commonInit();
        zygoteInitNative();

        int curArg = 0;
        for ( /* curArg */ ; curArg < argv.length; curArg++) {
            String arg = argv[curArg];

            if (arg.equals("--")) {
                curArg++;
                break;
            } else if (!arg.startsWith("--")) {
                break;
            } else if (arg.startsWith("--nice-name=")) {
                String niceName = arg.substring(arg.indexOf('=') + 1);
                Process.setArgV0(niceName);
            }
        }

        if (curArg == argv.length) {
            Slog.e(TAG, "Missing classname argument to RuntimeInit!");
            // let the process exit
            return;
        }

        // Remaining arguments are passed to the start class's static main

        String startClass = argv[curArg++];
        String[] startArgs = new String[argv.length - curArg];

        System.arraycopy(argv, curArg, startArgs, 0, startArgs.length);
        invokeStaticMain(startClass, startArgs);
    }

    public static final native void zygoteInitNative();

    /**
     * Returns 1 if the computer is on. If the computer isn't on, the value returned by this method is undefined.
     */
    public static final native int isComputerOn();

    /**
     * Turns the computer on if the computer is off. If the computer is on, the behavior of this method is undefined.
     */
    public static final native void turnComputerOn();

    /**
     *
     * @return 1 if the device has a qwerty keyboard
     */
    public static native int getQwertyKeyboard();

    /**
     * Report a serious error in the current process.  May or may not cause
     * the process to terminate (depends on system settings).
     *
     * @param tag to record with the error
     * @param t exception describing the error site and conditions
     */
    public static void wtf(String tag, Throwable t) {
        try {
            if (ActivityManagerNative.getDefault().handleApplicationWtf(
                    mApplicationObject, tag, new ApplicationErrorReport.CrashInfo(t))) {
                // The Activity Manager has already written us off -- now exit.
                Process.killProcess(Process.myPid());
                System.exit(10);
            }
        } catch (Throwable t2) {
            Slog.e(TAG, "Error reporting WTF", t2);
        }
    }

    /** Counter used to prevent reentrancy in {@link #reportException}. */
    private static final AtomicInteger sInReportException = new AtomicInteger();

    /**
     * Set the object identifying this application/process, for reporting VM
     * errors.
     */
    public static final void setApplicationObject(IBinder app) {
        mApplicationObject = app;
    }

    public static final IBinder getApplicationObject() {
        return mApplicationObject;
    }

    /**
     * Enable debugging features.
     */
    static {
        // Register handlers for DDM messages.
        android.ddm.DdmRegister.registerHandlers();
    }
}
