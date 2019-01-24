# Faketime

This native Java Agent allows you to change the time in a Java program, without affecting the system clock. It allows control over the `System.currentTimeMillis()` method, which is used by most other time-related functionality provided by the JVM.

## How to use it

### Step 1: Get the library:

Use one of these options:

<hr>

#### Option 1: For Mac OS X: (the easy way)

Download [libfaketime.jnilib](https://github.com/arvindsv/faketime/blob/master/lib/mac/libfaketime.jnilib?raw=true) into some location.

OR

#### Option 2: For Max OS X (the "hard" way)

Run this, on a clone of this repository:

```
gcc -shared -I $JAVA_HOME/include -Wall src/FakeTimeAgent.c -o libfaketime.jnilib
```

OR

#### Option 3: For Linux (64-bit)

Run this, on a clone of this repository:

```
gcc -fPIC -shared -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -m64 -Wall src/FakeTimeAgent.c -o libfaketime.so
```

OR

#### Option 4: For Linux (32-bit)

Run this, on a clone of this repository:

```
gcc -fPIC -shared -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -m32 -Wall src/FakeTimeAgent.c -o libfaketime.so
```

<hr>

### Step 2: Use it:

* Run your Java program (say, org.test.Main) with these agent-specific extra arguments (see [issue #3](https://github.com/arvindsv/faketime/issues/3)), like this:

```
java -agentpath:/path/to/the/library/you/got/above \
  -XX:+UnlockDiagnosticVMOptions \
  -XX:DisableIntrinsic=_currentTimeMillis \
  -XX:CompileCommand=exclude,java/lang/System.currentTimeMillis \
  org.test.Main
```

* In your Java code, you can set the property **faketime.offset.seconds** to the number of *seconds* you want the time altered by. For example, to add a day, you can do something like this:

```
System.setProperty("faketime.offset.seconds", "86400");
```
        
* That's it! Take a look at [FakeTimeTest.java](https://github.com/arvindsv/faketime/blob/master/FakeTimeTest.java) if you need to see some Java code which uses it.
