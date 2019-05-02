#!/bin/sh
set -eu

gcc -shared -I $JAVA_HOME/include -Wall src/FakeTimeAgent.c -o libfaketime.jnilib 
mkdir -p lib/mac/ && mv libfaketime.jnilib lib/mac/

javac FakeTimeTest.java 
javac FakeTimeFileTest.java

java -agentpath:./lib/mac/libfaketime.jnilib FakeTimeTest
java -agentpath:./lib/mac/libfaketime.jnilib FakeTimeFileTest
