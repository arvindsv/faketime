# Faketime

This native Java Agent allows you to change the time in a Java program, without affecting the system clock. It allows control over the System.currentTimeMillis() method, which is used by most other time-related functionality provided by the JVM.

## How to use it

### The easy way - For the Mac

* Download [libfaketime.jnilib](https://github.com/arvindsv/faketime/blob/master/lib/mac/libfaketime.jnilib?raw=true) into some directory, say, /path/to/libfaketime.jnilib.

* Run your Java program (say, org.test.Main) with the agent-specific extra arguments, like this:

        java -agentpath:/path/to/libfaketime.jnilib org.test.Main

* In your Java code, you can set the property **faketime.offset.seconds** to the number of *seconds* you want the time altered by. For example, to add a day, you can do something like this:

        System.setProperty("faketime.offset.seconds", String.valueOf(86400));
        
* That's it! Take a look at [FakeTimeTest.java](https://github.com/arvindsv/faketime/blob/master/FakeTimeTest.java) if you need to see some Java code which uses it.

### The "hard" way - For the Mac

* Clone this repo

* Run this:

        gcc -shared -I $JAVA_HOME/include -Wall src/FakeTimeAgent.c -o libfaketime.jnilib

* This will create libfaketime.jnilib in the current directory. Run your Java program (say, org.test.Main) with the agent-specific extra arguments, like this:

        java -agentpath:/path/to/libfaketime.jnilib org.test.Main

    or, if the current directory has libfaketime.jnilib, you can use: 

        java -agentlib:faketime org.test.Main

* In your Java code, you can set the property **faketime.offset.seconds** to the number of *seconds* you want the time altered by. For example, to add a day, you can do something like this:

        System.setProperty("faketime.offset.seconds", String.valueOf(86400 * 1000));

* That's it! Take a look at [FakeTimeTest.java](https://github.com/arvindsv/faketime/blob/master/FakeTimeTest.java) if you need to see some Java code which uses it.