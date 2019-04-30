import java.util.Calendar;
import java.lang.String;
import java.nio.file.Files;
import java.nio.file.Path;

public class FakeTimeFileTest {
  public static void main(String args[]) throws Throwable{
    final Path filePath = Files.createTempFile(null, null);
    System.out.println("Using temporary file " + filePath.toString());
    
    System.setProperty("faketime.offset.file", filePath.toAbsolutePath().toString());

    for(int i = 0; i < 10; i++) {
      Files.write(filePath, String.valueOf(i * -86400).getBytes("UTF-8"));

      System.out.println("Time: " + System.currentTimeMillis());
      Calendar calendar = Calendar.getInstance();
      System.out.println("Date: " + calendar.getTime());
    }
  }
}
