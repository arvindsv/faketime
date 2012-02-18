package org.time;

import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.security.ProtectionDomain;

public class TimeChangingTransformer implements ClassFileTransformer {
    @Override
    public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined, ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
        if (!"java/lang/System".equals(className)) {
            return null;
        }

        return null;
//        return changeTimeMethod(classfileBuffer);
    }

    public byte[] changeTimeMethod(byte[] classfileBuffer) {
        try {
            String timeMethodName = "currentTimeMillis";
            ClassWriter writer = new ClassWriter(0);
            ClassReader reader = new ClassReader(classfileBuffer);
            reader.accept(new MyClassAdapter(writer), 0);
            return writer.toByteArray();

/*
            ClassPool pool = ClassPool.getDefault();
            CtClass originalSystemClass = pool.makeClass(new ByteArrayInputStream(classfileBuffer));
            originalSystemClass.setName(renamedSystemClassName);
            originalSystemClass.setModifiers(Modifier.PUBLIC);
            originalSystemClass.getConstructor(Descriptor.ofConstructor(new CtClass[0])).setModifiers(Modifier.PROTECTED);

            CtClass newSystemClass = pool.makeClass(new ByteArrayInputStream(classfileBuffer));
            newSystemClass.setName("java.lang.System");

            CtMethod oldMethod = originalSystemClass.getDeclaredMethod(timeMethodName);
            CtMethod newMethod = CtNewMethod.copy(oldMethod, originalSystemClass, null);
            newMethod.setBody("return null;");
            newMethod.setModifiers(Modifier.PUBLIC);

            originalSystemClass.removeMethod(oldMethod);
            originalSystemClass.addMethod(newMethod);

            FileUtils.writeByteArrayToFile(new File("/tmp/oldSystem" + new Random().nextInt() + ".class"), originalSystemClass.toBytecode());

//            FileUtils.writeByteArrayToFile(new File("/tmp/theOneInCode.class"), IOUtils.toByteArray(getClass().getResourceAsStream("/java/lang/System.class")));
            FileUtils.writeByteArrayToFile(new File("/tmp/newSystem" + new Random().nextInt() + ".class"), originalSystemClass.toBytecode());

            return originalSystemClass.toBytecode();
*/
        } catch (Throwable e) {
            e.printStackTrace();
            throw new RuntimeException(e);
        }
    }
}
