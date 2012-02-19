import java.lang.reflect.Method;

public class SimpleThread {
  native static void check(Thread thr, ClassLoader cl);
  static MyThread t;

  public static void main(String args[]) throws Throwable{
    System.setProperty("faketime.offset", "500000000000");
    System.out.println("Time: " + System.currentTimeMillis());

    Method[] methods = System.class.getMethods();
    for (Method method : methods) {
      System.out.println(method.getName());
    }

    t = new MyThread();
    System.out.println("Creating and running 5 threads...");
    for(int i = 0; i < 5; i++) {
      Thread thr = new Thread(t,"MyThread"+i);
      thr.start();
      try {
        thr.join();
      } catch (Throwable t) {
      }
    }

  }
}

class MyThread implements Runnable {

  Thread t;

  public MyThread() {
  }

  public void run() {
    /* NO-OP */
    try {
      "a".getBytes("ASCII");
      excep();
      Thread.sleep(1000);
    } catch (java.lang.InterruptedException e){
      e.printStackTrace();
    } catch (Throwable t) {
    }
  }

  public void excep() throws Throwable{
    throw new Exception("Thread Exception from MyThread");
  }
}        



