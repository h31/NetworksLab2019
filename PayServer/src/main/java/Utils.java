import java.io.*;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.util.Scanner;

public class Utils {
    public static void addLineInFile(String str, String path) {
        try (FileWriter writer = new FileWriter(path, true)) {
            writer.write(str);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
