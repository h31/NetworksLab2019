package client;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Scanner;

public class ClientWriter extends Thread {
    private PrintWriter sock_pw;
    private Scanner scanner;

    public ClientWriter(PrintWriter sock_pw, Scanner scanner) {
        this.sock_pw = sock_pw;
        this.scanner = scanner;
    }

    @Override
    public void run() {
        String s;
        while (true) {
            System.out.print("> ");
            s = scanner.nextLine();
            if (s != null) {
                sock_pw.println(s);
            } else break;
        }
    }
}
