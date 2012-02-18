package org.time;

import com.sun.jdi.Bootstrap;
import com.sun.jdi.ReferenceType;
import com.sun.jdi.VirtualMachine;
import com.sun.jdi.VirtualMachineManager;
import com.sun.jdi.connect.spi.Connection;
import com.sun.jdi.connect.spi.TransportService;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class TransformerRegistrar {
    public static void main(String[] args) throws IOException {
        VirtualMachineManager virtualMachineManager = Bootstrap.virtualMachineManager();
        VirtualMachine virtualMachine = virtualMachineManager.createVirtualMachine(openJDIConnection());

        HashMap<ReferenceType, byte[]> map = new HashMap<ReferenceType, byte[]>();
        List<ReferenceType> referenceTypes = virtualMachine.classesByName("java.lang.System");
        for (ReferenceType referenceType : referenceTypes) {
            map.put(referenceType, FileUtils.readFileToByteArray(new File("/Users/aravind/tmp/run.sh")));
        }
        System.out.println(virtualMachine.canAddMethod());
        virtualMachine.redefineClasses(map);
//        virtualMachine.resume();
    }

    private static Connection openJDIConnection() throws IOException {
        TransportService ts;
        try {
            Class c = Class.forName("com.sun.tools.jdi.SocketTransportService");
            ts = (TransportService)c.newInstance();
        } catch (Exception x) {
            throw new Error(x);
        }

        return ts.attach("localhost:8000", 5000, 5000);    }

    public static void premain(String agentArguments, Instrumentation instrumentation) throws UnmodifiableClassException {
        if (true) return;
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
