package org.time;

import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;

public class TransformerRegistrar {
    public static void premain(String agentArguments, Instrumentation instrumentation) throws UnmodifiableClassException {
        instrumentation.addTransformer(new TimeChangingTransformer(), true);
        instrumentation.retransformClasses(System.class);
    }
}
