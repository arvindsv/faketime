#!/bin/sh
set -e

gcc -shared -I $JAVA_HOME/include -I $JAVA_HOME/include/darwin -Wall src/FakeTimeAgent.c -o libfaketime.jnilib
mkdir -p lib/mac/ && mv libfaketime.jnilib lib/mac/

javac FakeTimeTest.java 
java -agentpath:./lib/mac/libfaketime.jnilib FakeTimeTest
