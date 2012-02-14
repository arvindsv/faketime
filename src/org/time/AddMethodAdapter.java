package org.time;

import com.sun.xml.internal.ws.org.objectweb.asm.*;

public class AddMethodAdapter implements ClassVisitor {
    private final ClassWriter writer;

    @Override
    public void visitEnd() {
        MethodVisitor newMethod = visitMethod(Opcodes.ACC_PUBLIC, "run", "()V", null, null);
        if (newMethod != null) {
            newMethod.visitEnd();
        }
        writer.visitEnd();
    }

    public AddMethodAdapter(ClassWriter writer) {
        this.writer = writer;
    }

    @Override
    public void visit(int i, int i1, String s, String s1, String s2, String[] strings) {
        writer.visit(i, i1, s, s1, s2, strings);
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

    @Override
    public MethodVisitor visitMethod(int i, String s, String s1, String s2, String[] strings) {
        return writer.visitMethod(i, s, s1, s2, strings);
    }
}
