import com.android.build.api.instrumentation.AsmClassVisitorFactory
import com.android.build.api.instrumentation.ClassContext
import com.android.build.api.instrumentation.ClassData
import com.android.build.api.instrumentation.InstrumentationParameters
import org.objectweb.asm.ClassVisitor
import org.objectweb.asm.MethodVisitor
import org.objectweb.asm.Opcodes
import org.objectweb.asm.Opcodes.ASM9

private const val DESUGAR_CLASS_NAME = "com.topjohnwu.magisk.core.utils.Desugar"
private const val ZIP_ENTRY_CLASS_NAME = "java.util.zip.ZipEntry"
private const val ZIP_OUT_STREAM_CLASS_NAME = "org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream"
private const val ZIP_UTIL_CLASS_NAME = "org/apache/commons/compress/archivers/zip/ZipUtil"
private const val ZIP_ENTRY_GET_TIME_DESC = "()Ljava/nio/file/attribute/FileTime;"
private const val DESUGAR_GET_TIME_DESC =
    "(Ljava/util/zip/ZipEntry;)Ljava/nio/file/attribute/FileTime;"

private fun ClassData.isTypeOf(name: String) = className == name || superClasses.contains(name)

abstract class DesugarClassVisitorFactory : AsmClassVisitorFactory<InstrumentationParameters.None> {
    override fun createClassVisitor(
        classContext: ClassContext,
        nextClassVisitor: ClassVisitor
    ): ClassVisitor {
        return if (classContext.currentClassData.className == ZIP_OUT_STREAM_CLASS_NAME) {
            ZipEntryPatcher(classContext, ZipOutputStreamPatcher(nextClassVisitor))
        } else {
            ZipEntryPatcher(classContext, nextClassVisitor)
        }
    }

    override fun isInstrumentable(classData: ClassData) = classData.className != DESUGAR_CLASS_NAME

    // Patch ALL references to ZipEntry#getXXXTime
    class ZipEntryPatcher(
        private val classContext: ClassContext,
        cv: ClassVisitor
    ) : ClassVisitor(ASM9, cv) {
        override fun visitMethod(
            access: Int,
            name: String?,
            descriptor: String?,
            signature: String?,
            exceptions: Array<out String>?
        ) = MethodPatcher(super.visitMethod(access, name, descriptor, signature, exceptions))

        inner class MethodPatcher(mv: MethodVisitor?) : MethodVisitor(ASM9, mv) {
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

    // Patch ZipArchiveOutputStream#copyFromZipInputStream
    class ZipOutputStreamPatcher(cv: ClassVisitor) : ClassVisitor(ASM9, cv) {
        override fun visitMethod(
            access: Int,
            name: String,
            descriptor: String,
            signature: String?,
            exceptions: Array<out String?>?
        ): MethodVisitor? {
            return if (name == "copyFromZipInputStream") {
                MethodPatcher(super.visitMethod(access, name, descriptor, signature, exceptions))
            } else {
                super.visitMethod(access, name, descriptor, signature, exceptions)
            }
        }

        class MethodPatcher(mv: MethodVisitor?) : MethodVisitor(ASM9, mv) {
            override fun visitMethodInsn(
                opcode: Int,
                owner: String,
                name: String,
                descriptor: String?,
                isInterface: Boolean
            ) {
                if (owner == ZIP_UTIL_CLASS_NAME && name == "checkRequestedFeatures") {
                    // Redirect
                    mv.visitMethodInsn(
                        Opcodes.INVOKESTATIC,
                        DESUGAR_CLASS_NAME.replace('.', '/'),
                        name,
                        descriptor,
                        false
                    )
                } else {
                    super.visitMethodInsn(opcode, owner, name, descriptor, isInterface)
                }
            }
        }
    }
}
