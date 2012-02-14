class="$1"

java -classpath lib/asm-4.0.jar:lib/asm-util-4.0.jar org.objectweb.asm.util.ASMifier "$class" | indent -i4
