package org.time;

import javassist.*;
import org.apache.commons.io.FileUtils;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.security.ProtectionDomain;
import java.util.Random;

public class TimeChangingTransformer implements ClassFileTransformer {
    @Override
    public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined, ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
//        System.out.println(className);
        if (!"java/lang/System".equals(className)) {
            return null;
        }

        return changeTimeMethod(classfileBuffer);
    }

    public byte[] changeTimeMethod(byte[] classfileBuffer) {
        try {
//            FileUtils.writeByteArrayToFile(new File("/tmp/realSystem" + new Random().nextInt() + ".class"), classfileBuffer);

            String timeMethodName = "currentTimeMillis";
            String renamedSystemClassName = "java.lang.System$ORIGINAL";

            ClassPool pool = ClassPool.getDefault();
            CtClass originalSystemClass = pool.makeClass(new ByteArrayInputStream(classfileBuffer));
/*
            originalSystemClass.setName(renamedSystemClassName);
            originalSystemClass.setModifiers(Modifier.PUBLIC);
            originalSystemClass.getConstructor(Descriptor.ofConstructor(new CtClass[0])).setModifiers(Modifier.PROTECTED);

            CtClass newSystemClass = pool.makeClass(new ByteArrayInputStream(classfileBuffer));
            newSystemClass.setName("java.lang.System");
*/
            CtMethod oldMethod = originalSystemClass.getDeclaredMethod(timeMethodName);
            CtMethod newMethod = CtNewMethod.copy(oldMethod, originalSystemClass, null);
            newMethod.setBody("return null;");
            newMethod.setModifiers(Modifier.PUBLIC);

            originalSystemClass.removeMethod(oldMethod);
            originalSystemClass.addMethod(newMethod);

/*
            FileUtils.writeByteArrayToFile(new File("/tmp/oldSystem" + new Random().nextInt() + ".class"), originalSystemClass.toBytecode());
*/
//            FileUtils.writeByteArrayToFile(new File("/tmp/theOneInCode.class"), IOUtils.toByteArray(getClass().getResourceAsStream("/java/lang/System.class")));
            FileUtils.writeByteArrayToFile(new File("/tmp/newSystem" + new Random().nextInt() + ".class"), originalSystemClass.toBytecode());

            return originalSystemClass.toBytecode();
        } catch (Throwable e) {
            System.out.println(e);
            throw new RuntimeException(e);
        }
    }
}
