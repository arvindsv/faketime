package org.time.fake;

import javassist.ClassPool;
import javassist.CtClass;
import javassist.CtMethod;
import javassist.Loader;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;

public class Run {
    public static void Xmain(String[] args) throws Exception {
        CtClass ctClass = ClassPool.getDefault().get("java.lang.System");
        CtMethod setPropertyMethod = ctClass.getDeclaredMethod("setProperty");

        ctClass.removeMethod(setPropertyMethod);
        setPropertyMethod.setBody("return \"Hello\";");
        ctClass.addMethod(setPropertyMethod);

        FileUtils.writeByteArrayToFile(new File("/tmp/MySystem.class"), ctClass.toBytecode());
    }

    public static void main(String[] args) throws Throwable {
        if (args.length < 1) {
            System.err.println("Ouch! Usage: Run main-class args...");
            System.setProperty(null, null);
            return;
        }

        runMainInClassOfArgument(setupClassLoader(), args);

        if (true) return;
        VerboseTranslator translator = new VerboseTranslator();
        ClassPool pool = ClassPool.getDefault();
        Loader loader = new Loader(pool);
        loader.addTranslator(pool, translator);

        String[] pargs = new String[args.length - 1];
        System.arraycopy(args, 1, pargs, 0, pargs.length);
        loader.run(args[0], pargs);
    }

    private static ClassLoader setupClassLoader() throws Exception {
        ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
        URL[] urls = new URL[]{new File(".").toURI().toURL()};
        if (systemClassLoader instanceof URLClassLoader) {
            URLClassLoader urlClassLoader = (URLClassLoader) systemClassLoader;
            urls = urlClassLoader.getURLs();
        }
        return new VerboseLoader(urls, systemClassLoader.getParent());
    }

    private static void runMainInClassOfArgument(ClassLoader classLoader, String[] args) {
        try {
            Class clas = classLoader.loadClass(args[0]);
            Class[] ptypes = new Class[]{args.getClass()};

            Method main = clas.getDeclaredMethod("main", ptypes);
            String[] pargs = new String[args.length - 1];
            System.arraycopy(args, 1, pargs, 0, pargs.length);
            main.invoke(null, new Object[]{pargs});
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }
    }
}