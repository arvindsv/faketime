package org.time.fake;

import javassist.CtClass;
import javassist.Modifier;

public class Run {
    public static void main(String[] args) throws Exception {
        CtClass systemClass = javassist.ClassPool.getDefault().get("java.lang.System");

        systemClass.addMethod(javassist.CtNewMethod.make(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE, CtClass.voidType, "setTimeOffset", new CtClass[] { CtClass.longType }, new CtClass[0], null, systemClass));
        systemClass.addMethod(javassist.CtNewMethod.make(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE, CtClass.longType, "getTimeOffset", new CtClass[0], new CtClass[0], null, systemClass));

        byte[] bytes = systemClass.toBytecode();
        System.out.write(bytes, 0, bytes.length);
    }
}