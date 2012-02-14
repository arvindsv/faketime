package org.time;

import javassist.ClassPool;
import javassist.CtClass;
import javassist.CtMethod;
import javassist.CtNewMethod;

import java.io.ByteArrayInputStream;
import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.security.ProtectionDomain;

public class TimeChangingTransformer implements ClassFileTransformer {
    @Override
    public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined, ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
        if (!"java/lang/System".equals(className)) {
            return null;
        }

        try {
            String originalName = "currentTimeMillis";
            String renamedMethodName = "currentTimeMillis$original";

            CtClass ctClass = ClassPool.getDefault().makeClass(new ByteArrayInputStream(classfileBuffer));
            CtMethod oldMethod = ctClass.getDeclaredMethod(originalName);
            oldMethod.setName(renamedMethodName);

            CtMethod newMethod = CtNewMethod.copy(oldMethod, originalName, ctClass, null);

            StringBuffer body = new StringBuffer();
            body.append("{");
            body.append("out.println(\"   GOT IT!\");");
            body.append("long result = " + renamedMethodName + "();");
            body.append("out.println(\"   DONE!\");");
            body.append("return result;");
            body.append("}");

            newMethod.setBody(body.toString());
            ctClass.addMethod(newMethod);
            return ctClass.toBytecode();

        } catch (Throwable e) {
            System.out.println(e);
            throw new RuntimeException(e);
        }
    }
}
