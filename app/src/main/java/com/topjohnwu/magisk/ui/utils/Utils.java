package com.topjohnwu.magisk.ui.utils;

import org.sufficientlysecure.rootcommands.Shell;
import org.sufficientlysecure.rootcommands.command.SimpleCommand;

import java.io.IOException;
import java.util.concurrent.TimeoutException;

public class Utils {

    public static String executeCommand(String... commands) {
        try {
            Shell shell = Shell.startRootShell();
            SimpleCommand command = new SimpleCommand(commands);
            shell.add(command).waitForFinish();

            String output = command.getOutput();
            output = output.replaceAll("\n", "");

            shell.close();

            return output;
        } catch (IOException | TimeoutException e) {
            return "";
        }
    }

}
