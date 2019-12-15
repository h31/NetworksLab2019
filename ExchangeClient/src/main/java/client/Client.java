package client;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Scanner;

public class Client {
    private static final int port = 8080;
    private Scanner scanner;
    private PrintWriter sock_pw;
    private BufferedReader sock_br;

    public void run() throws IOException {
        Socket sock = new Socket("localhost", port);
        scanner = new Scanner(System.in);
        sock_br = new BufferedReader(new InputStreamReader(sock.getInputStream()));
        sock_pw = new PrintWriter(sock.getOutputStream(), true);
        System.out.println("Connection established");
        String s;

        enter();

        Thread clientWriter = new ClientWriter(sock_pw, scanner);
        clientWriter.start();

        while ((s = sock_br.readLine()) != null) {
            if (s.equals("exit")) return;
            System.out.println("\r" + s);
            System.out.print("> ");
        }
        sock.close();
        System.out.println("Server is shutdown");
        System.exit(0);
    }

    private void enter() throws IOException {
        String s;
        while (true) {
            System.out.println("Sign in (1) or sign up(2)?");
            s = scanner.nextLine();
            switch (s) {
                case "1":
                    System.out.print("Login: ");
                    s = scanner.nextLine();
                    sock_pw.println("signin " + s);
                    s = sock_br.readLine();
                    System.out.println(s);
                    if(s.equals("Enter complete")) {
                        System.out.println("Write \"help\" for help");
                        return;
                    }
                    else break;
                case "2":
                    System.out.print("Create login: ");
                    s = scanner.nextLine();
                    sock_pw.println("signup " + s);
                    s = sock_br.readLine();
                    System.out.println(s);
                    if(s.equals("Enter complete")) {
                        System.out.println("Write \"help\" for help");
                        return;
                    }
                    else break;
                default:
                    break;
            }
        }
    }

    public static void main(String[] args) throws IOException{
        new Client().run();
    }
}
