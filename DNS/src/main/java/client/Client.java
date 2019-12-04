package client;


import dnsPackage.parts.Header;
import dnsPackage.parts.Query;
import dnsPackage.utilits.PackageBuilder;

import java.io.IOException;
import java.net.*;

public class Client {
    private DatagramSocket socket;
    private InetAddress address;
    private int port = 53;
    private byte[] googleDnsServ = {8, 8, 8, 8};

    public Client() {
        try {
            socket = new DatagramSocket();
        } catch (SocketException e) {
            e.printStackTrace();
        }
        try {
            address = InetAddress.getByAddress(googleDnsServ);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
    }

    public void send(byte[] buf) {
        DatagramPacket packet = new DatagramPacket(buf, buf.length, address, port);
        try {
            socket.send(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void receive() {
        byte[] buf = new byte[512];
        DatagramPacket packet = new DatagramPacket(buf, buf.length);
        try {
            socket.receive(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
        buf = packet.getData();
        String received = packageToString(buf);
        System.out.println("server: " + received);
    }

    private void close() {
        socket.close();
    }

    private String packageToString(byte[] bytes) {
        StringBuilder address = new StringBuilder();
        for (int i = 0; i < 50; i++) {
            address.append((bytes[i])).append(" \n");
        }
        return address.toString();
    }

    public static void main(String[] args) {
        Client client = new Client();
        Header header = Header.getDefaultQueryHeader();
        Query query = new Query("facebook.com");
        client.send(PackageBuilder.makePackage(header, query));
        client.receive();
        client.close();
    }
}
