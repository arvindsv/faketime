package org.time.fake;

import javassist.CannotCompileException;
import javassist.ClassPool;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;

public class VerboseLoader extends URLClassLoader {
    public VerboseLoader(URL[] urls, ClassLoader parent) {
        super(urls, parent);
    }

    @Override
    protected Class<?> findClass(String name) throws ClassNotFoundException {
        System.out.println("Finding class: " + name);
        return super.findClass(name);
    }

    @Override
    public Class<?> loadClass(String name) throws ClassNotFoundException {
        String systemClassName = "java.lang.System";

        System.out.println("Loading class: " + name);
        if (!systemClassName.equals(name)) {
            return super.loadClass(name);
        }

        try {
            ClassPool pool = ClassPool.getDefault();
            return pool.makeClass(new FileInputStream(new File("/tmp/MySystem.class"))).toClass();
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (CannotCompileException e) {
            throw new RuntimeException(e);
        }
    }
}
