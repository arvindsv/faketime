import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.time.ClassTransformer;

import java.io.File;
import java.io.InputStream;

public class ClassTransformerTest {
    @Test
    public void shouldBoo() throws Throwable {
        InputStream stream = getClass().getResourceAsStream("/java/lang/System.class");
        FileUtils.writeByteArrayToFile(new File("/tmp/sys.class"), IOUtils.toByteArray(stream));
        new ClassTransformer().boo3();
    }
}
