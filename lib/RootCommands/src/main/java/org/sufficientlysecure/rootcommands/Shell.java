/*
 * Copyright (C) 2012 Dominik Schürmann <dominik@dominikschuermann.de>
 * Copyright (c) 2012 Stephen Erickson, Chris Ravenscroft, Adam Shanks, Jeremy Lakeman (RootTools)
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

package org.sufficientlysecure.rootcommands;

import org.sufficientlysecure.rootcommands.command.Command;
import org.sufficientlysecure.rootcommands.util.Log;
import org.sufficientlysecure.rootcommands.util.RootAccessDeniedException;
import org.sufficientlysecure.rootcommands.util.Utils;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class Shell implements Closeable {
    private static final String LD_LIBRARY_PATH = System.getenv("LD_LIBRARY_PATH");
    private static final String token = "F*D^W@#FGF";
    private final Process shellProcess;
    private final BufferedReader stdOutErr;
    private final DataOutputStream outputStream;
    private final List<Command> commands = new ArrayList<Command>();
    private boolean close = false;
    private Runnable inputRunnable = new Runnable() {
        public void run() {
            try {
                writeCommands();
            } catch (IOException e) {
                Log.e(RootCommands.TAG, "IO Exception", e);
            }
        }
    };
    private Runnable outputRunnable = new Runnable() {
        public void run() {
            try {
                readOutput();
            } catch (IOException e) {
                Log.e(RootCommands.TAG, "IOException", e);
            } catch (InterruptedException e) {
                Log.e(RootCommands.TAG, "InterruptedException", e);
            }
        }
    };

    private Shell(String shell, ArrayList<String> customEnv, String baseDirectory)
            throws IOException {
        Log.d(RootCommands.TAG, "Starting shell: " + shell);

        // start shell process!
        shellProcess = Utils.runWithEnv(shell, customEnv, baseDirectory);

        // StdErr is redirected to StdOut, defined in Command.getCommand()
        stdOutErr = new BufferedReader(new InputStreamReader(shellProcess.getInputStream()));
        outputStream = new DataOutputStream(shellProcess.getOutputStream());

        outputStream.write("echo Started\n".getBytes());
        outputStream.flush();

        while (true) {
            String line = stdOutErr.readLine();
            if (line == null)
                throw new RootAccessDeniedException(
                        "stdout line is null! Access was denied or this executeable is not a shell!");
            if ("".equals(line))
                continue;
            if ("Started".equals(line))
                break;

            destroyShellProcess();
            throw new IOException("Unable to start shell, unexpected output \"" + line + "\"");
        }

        new Thread(inputRunnable, "Shell Input").start();
        new Thread(outputRunnable, "Shell Output").start();
    }

    /**
     * Start root shell
     *
     * @param customEnv
     * @param baseDirectory
     * @return
     * @throws IOException
     */
    public static Shell startRootShell(ArrayList<String> customEnv, String baseDirectory)
            throws IOException {
        Log.d(RootCommands.TAG, "Starting Root Shell!");

        // On some versions of Android (ICS) LD_LIBRARY_PATH is unset when using su
        // We need to pass LD_LIBRARY_PATH over su for some commands to work correctly.
        if (customEnv == null) {
            customEnv = new ArrayList<String>();
        }
        customEnv.add("LD_LIBRARY_PATH=" + LD_LIBRARY_PATH);

        Shell shell = new Shell(Utils.getSuPath(), customEnv, baseDirectory);

        return shell;
    }

    /**
     * Start root shell without custom environment and base directory
     *
     * @return
     * @throws IOException
     */
    public static Shell startRootShell() throws IOException {
        return startRootShell(null, null);
    }

    /**
     * Start default sh shell
     *
     * @param customEnv
     * @param baseDirectory
     * @return
     * @throws IOException
     */
    public static Shell startShell(ArrayList<String> customEnv, String baseDirectory)
            throws IOException {
        Log.d(RootCommands.TAG, "Starting Shell!");
        Shell shell = new Shell("sh", customEnv, baseDirectory);
        return shell;
    }

    /**
     * Start default sh shell without custom environment and base directory
     *
     * @return
     * @throws IOException
     */
    public static Shell startShell() throws IOException {
        return startShell(null, null);
    }

    /**
     * Start custom shell defined by shellPath
     *
     * @param shellPath
     * @param customEnv
     * @param baseDirectory
     * @return
     * @throws IOException
     */
    public static Shell startCustomShell(String shellPath, ArrayList<String> customEnv,
                                         String baseDirectory) throws IOException {
        Log.d(RootCommands.TAG, "Starting Custom Shell!");
        Shell shell = new Shell(shellPath, customEnv, baseDirectory);

        return shell;
    }

    /**
     * Start custom shell without custom environment and base directory
     *
     * @param shellPath
     * @return
     * @throws IOException
     */
    public static Shell startCustomShell(String shellPath) throws IOException {
        return startCustomShell(shellPath, null, null);
    }

    /**
     * Destroy shell process considering that the process could already be terminated
     */
    private void destroyShellProcess() {
        try {
            // Yes, this really is the way to check if the process is
            // still running.
            shellProcess.exitValue();
        } catch (IllegalThreadStateException e) {
            // Only call destroy() if the process is still running;
            // Calling it for a terminated process will not crash, but
            // (starting with at least ICS/4.0) spam the log with INFO
            // messages ala "Failed to destroy process" and "kill
            // failed: ESRCH (No such process)".
            shellProcess.destroy();
        }

        Log.d(RootCommands.TAG, "Shell destroyed");
    }

    /**
     * Writes queued commands one after another into the opened shell. After an execution a token is
     * written to seperate command output on read
     *
     * @throws IOException
     */
    private void writeCommands() throws IOException {
        try {
            int commandIndex = 0;
            while (true) {
                DataOutputStream out;
                synchronized (commands) {
                    while (!close && commandIndex >= commands.size()) {
                        commands.wait();
                    }
                    out = this.outputStream;
                }
                if (commandIndex < commands.size()) {
                    Command next = commands.get(commandIndex);
                    next.writeCommand(out);
                    String line = "\necho " + token + " " + commandIndex + " $?\n";
                    out.write(line.getBytes());
                    out.flush();
                    commandIndex++;
                } else if (close) {
                    out.write("\nexit 0\n".getBytes());
                    out.flush();
                    Log.d(RootCommands.TAG, "Closing shell");
                    shellProcess.waitFor();
                    out.close();
                    return;
                } else {
                    Thread.sleep(50);
                }
            }
        } catch (InterruptedException e) {
            Log.e(RootCommands.TAG, "interrupted while writing command", e);
        }
    }

    /**
     * Reads output line by line, seperated by token written after every command
     *
     * @throws IOException
     * @throws InterruptedException
     */
    private void readOutput() throws IOException, InterruptedException {
        Command command = null;

        // index of current command
        int commandIndex = 0;
        while (true) {
            String lineStdOut = stdOutErr.readLine();

            // terminate on EOF
            if (lineStdOut == null)
                break;

            if (command == null) {

                // break on close after last command
                if (commandIndex >= commands.size()) {
                    if (close)
                        break;
                    continue;
                }

                // get current command
                command = commands.get(commandIndex);
            }

            int pos = lineStdOut.indexOf(token);
            if (pos > 0) {
                command.processOutput(lineStdOut.substring(0, pos));
            }
            if (pos >= 0) {
                lineStdOut = lineStdOut.substring(pos);
                String fields[] = lineStdOut.split(" ");
                int id = Integer.parseInt(fields[1]);
                if (id == commandIndex) {
                    command.setExitCode(Integer.parseInt(fields[2]));

                    // go to next command
                    commandIndex++;
                    command = null;
                    continue;
                }
            }
            command.processOutput(lineStdOut);
        }
        Log.d(RootCommands.TAG, "Read all output");
        shellProcess.waitFor();
        stdOutErr.close();
        destroyShellProcess();

        while (commandIndex < commands.size()) {
            if (command == null) {
                command = commands.get(commandIndex);
            }
            command.terminated("Unexpected Termination!");
            commandIndex++;
            command = null;
        }
    }

    /**
     * Add command to shell queue
     *
     * @param command
     * @return
     * @throws IOException
     */
    public Command add(Command command) throws IOException {
        if (close)
            throw new IOException("Unable to add commands to a closed shell");
        synchronized (commands) {
            commands.add(command);
            // set shell on the command object, to know where the command is running on
            command.addedToShell(this, (commands.size() - 1));
            commands.notifyAll();
        }

        return command;
    }

    /**
     * Close shell
     *
     * @throws IOException
     */
    public void close() throws IOException {
        synchronized (commands) {
            this.close = true;
            commands.notifyAll();
        }
    }

    /**
     * Returns number of queued commands
     *
     * @return
     */
    public int getCommandsSize() {
        return commands.size();
    }

}