import java.io.File

object FileSystem {
  val defaultDir = new File("./files")
}

object FileImplicitUtils {
  implicit class RichFile(file: File) extends File(file.getName) {
    def /(other: String): File = new File(file.getAbsolutePath + s"/$other")
  }
}
