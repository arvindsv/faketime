javac FakeTimeTest.java && gcc -shared -I $JAVA_HOME/include -Wall src/FakeTimeAgent.c -o libfaketime.jnilib && \
   mv libfaketime.jnilib lib/mac && java -agentpath:./lib/mac/libfaketime.jnilib FakeTimeTest
