package org.time;

import javassist.*;
import javassist.bytecode.Descriptor;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class ClassTransformer {
    public void boo() throws IOException, NotFoundException, CannotCompileException {
        String originalName = "xyz";
        String renamedMethodName = "xyz$original";

        File file = new File("/Users/aravind/scratch/projects/faketime/out/production/faketime/org/time/Abc.class");

        CtClass ctClass = ClassPool.getDefault().makeClass(new FileInputStream(file));
        CtMethod oldMethod = ctClass.getDeclaredMethod("xyz");
        oldMethod.setName(renamedMethodName);

        CtMethod newMethod = CtNewMethod.copy(oldMethod, originalName, ctClass, null);

        StringBuffer body = new StringBuffer();
        body.append("{");
        body.append("System.out.println(\"Hello!\");");
        body.append(renamedMethodName + "();");
        body.append("System.out.println(\"Done!\");");
        body.append("}");

        newMethod.setBody(body.toString());
        ctClass.addMethod(newMethod);

        FileUtils.writeByteArrayToFile(new File("/tmp/org/time/Abc.class"), ctClass.toBytecode());
/*
        byte[] bytes = FileUtils.readFileToByteArray(file);
        ClassReader reader = new ClassReader(bytes);
        ClassWriter writer = new ClassWriter(reader, 0);

        reader.accept(new AddMethodAdapter(writer), 0);
        byte[] output = writer.toByteArray();
        FileUtils.writeByteArrayToFile(new File("/tmp/Abc.class"), output);
*/
    }

    public void boo2() throws Throwable {
        String originalName = "currentTimeMillis";

        ClassPool pool = ClassPool.getDefault();
        CtClass ctClass = pool.get("java.lang.System");
        ctClass.setName("java.lang.System$ORIGINAL");
        ctClass.setModifiers(Modifier.PUBLIC);
        ctClass.getConstructor(Descriptor.ofConstructor(new CtClass[0])).setModifiers(Modifier.PROTECTED);

        CtClass newSystemClass = pool.makeClass("java.lang.System", ctClass);

        CtMethod newMethod = new CtMethod(CtClass.longType, "currentTimeMillis", new CtClass[0], newSystemClass);
        newMethod.setBody("return java.lang.System$ORIGINAL.currentTimeMillis();");
        newSystemClass.addMethod(newMethod);

        CtMethod method = ctClass.getDeclaredMethod(originalName);

        CodeConverter converter = new CodeConverter();
        converter.redirectMethodCall(originalName, method);
        method.instrument(converter);

        FileUtils.writeByteArrayToFile(new File("/tmp/System.class"), newSystemClass.toBytecode());
    }

    public void boo3() throws Throwable {
        String originalName = "currentTimeMillis";

        ClassPool pool = ClassPool.getDefault();
        CtClass ctClass = pool.get("java.lang.System");

/*
        CtMethod newMethod = new CtMethod(CtClass.longType, "currentTimeMillis", new CtClass[0], ctClass);
        newMethod.setModifiers(Modifier.PUBLIC);
        newMethod.setBody("return 0;");
*/

/*
        CtMethod method = ctClass.getDeclaredMethod(originalName);
        CodeConverter converter = new CodeConverter();
        converter.redirectMethodCall(originalName, method);
        method.instrument(converter);
*/

        FileUtils.writeByteArrayToFile(new File("/tmp/System.class"), ctClass.toBytecode());
    }
}
