package server;

import java.io.*;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.util.Scanner;

public class Utils {
    public static synchronized void addLineInFile(String str, String path) {
        try (FileWriter writer = new FileWriter(path, true)) {
            writer.write(str);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static synchronized void updateUserSizeInfo(User user, String path) throws IOException {
        List<String> lines = Files.readAllLines(Paths.get(path));
        for (int i = 0; i < lines.size(); i++) {
            String[] strings = lines.get(i).split("\\s+");
            if (strings[0].equals(user.getLogin())) {
                strings[2] = Integer.toString(user.getSize());
                StringBuilder stringBuilder = new StringBuilder();
                for (String str : strings) stringBuilder.append(str).append(" ");
                lines.set(i, stringBuilder.toString());
            }
        }
        Files.write(Paths.get(path), lines);
    }
}
