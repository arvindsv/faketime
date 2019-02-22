#!/bin/sh
set -eu

gcc -shared -I $JAVA_HOME/include -I $JAVA_HOME/include/linux -m32 -Wall src/FakeTimeAgent.c -o libfaketime.so 
mkdir -p lib/linux && mv libfaketime.so lib/linux/ 

javac FakeTimeTest.java 
java -agentpath:./lib/linux/libfaketime.so FakeTimeTest
