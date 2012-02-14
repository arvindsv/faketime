import javassist.CannotCompileException;
import javassist.NotFoundException;
import org.junit.Test;
import org.time.ClassTransformer;

import java.io.IOException;

public class ClassTransformerTest {
    @Test
    public void shouldBoo() throws IOException, NotFoundException, CannotCompileException {
        new ClassTransformer().boo();
    }
}
