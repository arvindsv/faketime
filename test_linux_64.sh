#!/bin/sh
set -eu

gcc -fPIC -shared -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -m64 -Wall src/FakeTimeAgent.c -o libfaketime.so 
mkdir -p lib/linux && mv libfaketime.so lib/linux/

javac FakeTimeTest.java
javac FakeTimeFileTest.java

java -agentpath:./lib/linux/libfaketime.so FakeTimeTest
java -agentpath:./lib/linux/libfaketime.so FakeTimeFileTest
