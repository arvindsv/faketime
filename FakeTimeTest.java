import java.util.Calendar;

public class FakeTimeTest {
  public static void main(String args[]) throws Throwable{
    for(int i = 0; i < 10; i++) {
      System.setProperty("faketime.offset.seconds", String.valueOf(i * 86400));

      System.out.println("Time: " + System.currentTimeMillis());
      Calendar calendar = Calendar.getInstance();
      System.out.println("Date: " + calendar.getTime());
    }

    for(int i = 0; i < 10; i++) {
      System.setProperty("faketime.fixed.seconds", String.valueOf(i * 86400));

      System.out.println("Time: " + System.currentTimeMillis());
      Calendar calendar = Calendar.getInstance();
      System.out.println("Date: " + calendar.getTime());
    }
  }
}
