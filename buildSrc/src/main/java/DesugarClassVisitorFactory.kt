import com.android.build.api.instrumentation.AsmClassVisitorFactory
import com.android.build.api.instrumentation.ClassContext
import com.android.build.api.instrumentation.ClassData
import com.android.build.api.instrumentation.InstrumentationParameters
import org.objectweb.asm.ClassVisitor
import org.objectweb.asm.MethodVisitor
import org.objectweb.asm.Opcodes
import org.objectweb.asm.Opcodes.ASM9

private const val DESUGAR_CLASS_NAME = "com/topjohnwu/magisk/core/utils/Desugar"
private const val ZIP_ENTRY_GET_TIME_DESC = "()Ljava/nio/file/attribute/FileTime;"
private const val DESUGAR_GET_TIME_DESC = "(Ljava/util/zip/ZipEntry;)Ljava/nio/file/attribute/FileTime;"

abstract class DesugarClassVisitorFactory : AsmClassVisitorFactory<InstrumentationParameters.None> {
    override fun createClassVisitor(
        classContext: ClassContext,
        nextClassVisitor: ClassVisitor
    ): ClassVisitor {
        return DesugarClassVisitor(nextClassVisitor)
    }

    override fun isInstrumentable(classData: ClassData) = true

    class DesugarClassVisitor(cv: ClassVisitor) : ClassVisitor(ASM9, cv) {
        override fun visitMethod(
            access: Int,
            name: String?,
            descriptor: String?,
            signature: String?,
            exceptions: Array<out String>?
        ): MethodVisitor {
            return DesugarMethodVisitor(super.visitMethod(access, name, descriptor, signature, exceptions))
        }
    }

    class DesugarMethodVisitor(mv: MethodVisitor?) : MethodVisitor(ASM9, mv) {
        override fun visitMethodInsn(
            opcode: Int,
            owner: String,
            name: String,
            descriptor: String,
            isInterface: Boolean
        ) {
            if (!process(name, descriptor)) {
                super.visitMethodInsn(opcode, owner, name, descriptor, isInterface)
            }
        }

        private fun process(name: String, descriptor: String): Boolean {
            if (descriptor != ZIP_ENTRY_GET_TIME_DESC)
                return false
            when (name) {
                "getLastModifiedTime", "getLastAccessTime", "getCreationTime" -> {
                    mv.visitMethodInsn(
                        Opcodes.INVOKESTATIC,
                        DESUGAR_CLASS_NAME,
                        name,
                        DESUGAR_GET_TIME_DESC,
                        false
                    )
                }
                else -> return false
            }
            return true
        }
    }
}
