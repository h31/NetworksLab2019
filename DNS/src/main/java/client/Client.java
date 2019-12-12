package client;


import dnsPackage.enams.RCode;
import dnsPackage.parts.Header;
import dnsPackage.parts.Query;
import dnsPackage.utilits.PackageBuilder;
import dnsPackage.utilits.PackageReader;

import java.io.IOException;
import java.net.*;
import java.util.Arrays;
import java.util.Scanner;

public class Client {
    private DatagramSocket socket;
    private InetAddress address;
    private int port;

    public Client() {
    }

    public Client init(String address, int port) {
        this.port = port;
        try {
            socket = new DatagramSocket();
        } catch (SocketException e) {
            e.printStackTrace();
        }
        try {
            this.address = InetAddress.getByName(address);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
        return this;
    }

    public void run() {
        PackageReader packageReader = new PackageReader();
        PackageBuilder packageBuilder = new PackageBuilder();
        Scanner scanner = new Scanner(System.in);
        while (true) {
            System.out.print("Enter: ");
            String input = scanner.nextLine();
            switch (input) {
                case "\\end":
                    close();
                    return;
                case "\\print":
                    System.out.println("Send package: \n" + packageBuilder.toString());
                    System.out.println("Receive package: \n" + packageReader.toString());
                    continue;
                default: {
                    packageBuilder = new PackageBuilder()
                            .addHeader(new Header().setDefQueryFlags())
                            .addQuery(new Query(input))
                            .build();
                    break;
                }
            }
            send(packageBuilder.getBytes());
            byte[] bytes = receive();
            packageReader.read(bytes);
            if (packageReader.getHeader().getFlags().getRCode() == RCode.NO_ERR) {
                System.out.println(packageReader.getAnswerData());
            }
        }
    }

    private void send(byte[] bytes) {
        DatagramPacket packet = new DatagramPacket(bytes, bytes.length, address, port);
        try {
            socket.send(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private byte[] receive() {
        byte[] bytes = new byte[512];
        DatagramPacket packet = new DatagramPacket(bytes, bytes.length);
        try {
            socket.receive(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return packet.getData();
    }

    private void close() {
        socket.close();
    }

    private String packageToString(byte[] bytes) {
        StringBuilder bytesString = new StringBuilder();
        bytesString.append("Bytes: ");
        bytesString.append(Arrays.toString(bytes));
        return bytesString.toString();
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Input dns server ip: ");
        String address = scanner.nextLine();
        System.out.print("Input port: ");
        int port = scanner.nextInt();

        Client client = new Client()
                .init(address, port);
                //.init("8.8.8.8", 53);
                //.init("127.0.0.1", 4444);
        client.run();
    }
}
