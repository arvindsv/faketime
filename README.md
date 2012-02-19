# Faketime

This native Java Agent allows you to change the time in a Java program, without affecting the system clock. It allows control over the System.currentTimeMillis() method, which is used by most other time-related functionality provided by the JVM.

## How to use it

### The easy way - For the Mac

* Download lib/mac/libfaketime.jnilib into some directory, say, /path/to/libfaketime.jnilib.
* Run your Java program (say, org.test.Main) with the agent-specific extra arguments, like this:

    java -agentpath:/path/to/libfaketime.jnilib org.test.Main

* In your program, you can set the property **faketime.offset** to the number of milliseconds you want the time altered by. For example, to add a day, you can do something like this:

    System.setProperty("faketime.offset", String.valueOf(i * 86400 * 1000));
