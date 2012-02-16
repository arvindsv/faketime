package org.time;

import org.objectweb.asm.*;

import static com.sun.xml.internal.ws.org.objectweb.asm.Opcodes.ACC_NATIVE;

public class MyClassAdapter extends ClassVisitor {

    private String className;
    private ClassWriter writer;

    public MyClassAdapter(ClassWriter writer) {
        super(Opcodes.ASM4, writer);
        this.writer = writer;
    }

    @Override
    public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
        this.className = name;
        writer.visit(version, access, name, signature, superName, interfaces);
    }

    @Override
    public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {

        boolean isNative = (access & ACC_NATIVE) != 0;
        if (isNative) {

            // Create the wrapper method with the same credentials, except from being native
            writer.newMethod(className, name, desc, false);
            MethodVisitor mv = writer.visitMethod(access ^ ACC_NATIVE, name, desc, signature, exceptions);
//            mv = new NativeMethodAdapter(mv, className, name, desc);

            // Create the prefixed method that will be linked to the actual implementation of the original method
            // the usage of setNativeMethodPrefix elsewhere is assumed here
            writer.visitMethod(access, "timeChangingAgent" + name, desc, signature, exceptions);
            return mv;
        } else {
            return writer.visitMethod(access, name, desc, signature, exceptions);
        }
    }

    @Override
    public void visitEnd() {
        writer.visitEnd();
    }

    @Override
    public void visitSource(String s, String s1) {
        writer.visitSource(s, s1);
    }

    @Override
    public void visitOuterClass(String s, String s1, String s2) {
        writer.visitOuterClass(s, s1, s2);
    }

    @Override
    public AnnotationVisitor visitAnnotation(String s, boolean b) {
        return writer.visitAnnotation(s, b);
    }

    @Override
    public void visitAttribute(Attribute attribute) {
        writer.visitAttribute(attribute);
    }

    @Override
    public void visitInnerClass(String s, String s1, String s2, int i) {
        writer.visitInnerClass(s, s1, s2, i);
    }

    @Override
    public FieldVisitor visitField(int i, String s, String s1, String s2, Object o) {
        return writer.visitField(i, s, s1, s2, o);
    }
}