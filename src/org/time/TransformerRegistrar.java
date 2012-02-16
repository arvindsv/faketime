package org.time;

import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;
import java.util.ArrayList;

public class TransformerRegistrar {
    public static void premain(String agentArguments, Instrumentation instrumentation) throws UnmodifiableClassException {
        TimeChangingTransformer transformer = new TimeChangingTransformer();
        instrumentation.addTransformer(transformer, true);
        instrumentation.setNativeMethodPrefix(transformer, "timeChangingAgent");
        Class[] allLoadedClasses = instrumentation.getAllLoadedClasses();
        ArrayList<Class> modifiableClasses = new ArrayList<Class>();

        for (Class loadedClass : allLoadedClasses) {
            if (instrumentation.isModifiableClass(loadedClass)) {
                modifiableClasses.add(loadedClass);
            }
        }
        instrumentation.retransformClasses(modifiableClasses.toArray(new Class<?>[modifiableClasses.size()]));
/*
        try {
            byte[] theClassFile = new TimeChangingTransformer().changeTimeMethod(IOUtils.toByteArray(TransformerRegistrar.class.getResourceAsStream("/java/lang/System.class"))) ;
            FileUtils.writeByteArrayToFile(new File("/tmp/gah.class"), theClassFile);
            instrumentation.redefineClasses(new ClassDefinition(System.class, theClassFile));
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
*/
        System.out.println("Be 0: " + System.currentTimeMillis());
    }
}
