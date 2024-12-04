import com.android.build.api.instrumentation.AsmClassVisitorFactory
import com.android.build.api.instrumentation.ClassContext
import com.android.build.api.instrumentation.ClassData
import com.android.build.api.instrumentation.InstrumentationParameters
import org.objectweb.asm.ClassVisitor
import org.objectweb.asm.MethodVisitor
import org.objectweb.asm.Opcodes
import org.objectweb.asm.Opcodes.ASM9

private const val ZIP_ENTRY_CLASS_NAME = "java.util.zip.ZipEntry"
private const val DESUGAR_CLASS_NAME = "com.topjohnwu.magisk.core.utils.Desugar"
private const val ZIP_ENTRY_GET_TIME_DESC = "()Ljava/nio/file/attribute/FileTime;"
private const val DESUGAR_GET_TIME_DESC =
    "(Ljava/util/zip/ZipEntry;)Ljava/nio/file/attribute/FileTime;"

private fun ClassData.isTypeOf(name: String) = className == name || superClasses.contains(name)

abstract class DesugarClassVisitorFactory : AsmClassVisitorFactory<InstrumentationParameters.None> {
    override fun createClassVisitor(
        classContext: ClassContext,
        nextClassVisitor: ClassVisitor
    ): ClassVisitor {
        return DesugarClassVisitor(classContext, nextClassVisitor)
    }

    override fun isInstrumentable(classData: ClassData) = classData.className != DESUGAR_CLASS_NAME

    class DesugarClassVisitor(private val classContext: ClassContext, cv: ClassVisitor) :
        ClassVisitor(ASM9, cv) {
        override fun visitMethod(
            access: Int,
            name: String?,
            descriptor: String?,
            signature: String?,
            exceptions: Array<out String>?
        ): MethodVisitor {
            return DesugarMethodVisitor(
                super.visitMethod(access, name, descriptor, signature, exceptions)
            )
        }

        inner class DesugarMethodVisitor(mv: MethodVisitor?) :
            MethodVisitor(ASM9, mv) {
            override fun visitMethodInsn(
                opcode: Int,
                owner: String,
                name: String,
                descriptor: String,
                isInterface: Boolean
            ) {
                if (!process(owner, name, descriptor)) {
                    super.visitMethodInsn(opcode, owner, name, descriptor, isInterface)
                }
            }

            private fun process(owner: String, name: String, descriptor: String): Boolean {
                val classData = classContext.loadClassData(owner.replace("/", ".")) ?: return false
                if (!classData.isTypeOf(ZIP_ENTRY_CLASS_NAME))
                    return false
                if (descriptor != ZIP_ENTRY_GET_TIME_DESC)
                    return false
                return when (name) {
                    "getLastModifiedTime", "getLastAccessTime", "getCreationTime" -> {
                        mv.visitMethodInsn(
                            Opcodes.INVOKESTATIC,
                            DESUGAR_CLASS_NAME.replace('.', '/'),
                            name,
                            DESUGAR_GET_TIME_DESC,
                            false
                        )
                        true
                    }
                    else -> false
                }
            }
        }
    }
}
