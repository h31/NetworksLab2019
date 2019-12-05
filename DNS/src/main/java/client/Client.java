package client;


import dnsPackage.enams.RCode;
import dnsPackage.parts.Header;
import dnsPackage.parts.Query;
import dnsPackage.utilits.PackageBuilder;
import dnsPackage.utilits.PackageReader;

import javax.crypto.interfaces.PBEKey;
import java.io.IOException;
import java.net.*;
import java.util.Scanner;

public class Client {
    private DatagramSocket socket;
    private InetAddress address;
    private int port = 53;
    private byte[] googleDnsServ = {8, 8, 8, 8};

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

    public void send(byte[] buf) {
        DatagramPacket packet = new DatagramPacket(buf, buf.length, address, port);
        try {
            socket.send(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public byte[] receive() {
        byte[] buf = new byte[512];
        DatagramPacket packet = new DatagramPacket(buf, buf.length);
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
        StringBuilder address = new StringBuilder();
        for (int i = 0; i < bytes.length; i++) {
            address.append((bytes[i])).append(" \n");
        }
        return address.toString();
    }

    public static void main(String[] args) {
        PackageReader packageReader = new PackageReader();
        PackageBuilder packageBuilder = new PackageBuilder();

        Scanner scanner = new Scanner(System.in);
        System.out.println("Input dns server ip: ");
        String address = scanner.nextLine();
        System.out.println("Input port: ");
        int port = scanner.nextInt();

        Client client = new Client()
                .init(address, port);
        scanner.nextLine();
        while (true) {
            System.out.println("Enter message:");
            String input = scanner.nextLine();
            switch (input) {
                case "\\end":
                    client.close();
                    return;
                case "\\print":
                    System.out.println("Send package: \n" + packageBuilder.toString());
                    System.out.println("Recive package: \n" + packageReader.toString());
                default: {
                    packageBuilder = new PackageBuilder()
                            .addHeader(new Header().setDefQueryFlags())
                            .addQuery(new Query(input))
                            .build();
                    break;
                }
            }
            client.send(packageBuilder.getBytes());
            byte[] bytes = client.receive();
            packageReader.read(bytes);
            if (packageReader.getHeader().getFlags().getRCode() == RCode.NO_ERR) {
                System.out.println(packageReader.getAnswerData());
            }
        }
    }
}
