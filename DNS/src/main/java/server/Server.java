package server;

import dnsPackage.enams.RRClass;
import dnsPackage.parts.Answer;
import dnsPackage.parts.Header;
import dnsPackage.utilits.PackageBuilder;
import dnsPackage.utilits.PackageReader;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.HashMap;

public class Server extends Thread {
    private DatagramSocket socket;
    private boolean running;
    private byte[] buf = new byte[512];
    private HashMap<String, String> adressMap = new HashMap<>();

    public Server() {
        try {
            socket = new DatagramSocket(4445);
        } catch (SocketException e) {
            e.printStackTrace();
        }

        adressMap.put("example.com.", "1.1.1.1");
    }

    public void run() {
        DatagramPacket packet
                = new DatagramPacket(buf, buf.length);

        //receive package
        try {
            socket.receive(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }

        PackageReader packageReader = new PackageReader();
        packageReader.read(packet.getData());

        InetAddress address = packet.getAddress();
        int port = packet.getPort();

        PackageBuilder packageBuilder = new PackageBuilder()
                .addHeader(new Header())
                .addQuery(packageReader.getQuery())
                .addAnswer(new Answer().makeATypeAnswer(12, 123, RRClass.IN,"228.228.14.88"))
                .build();
        buf = packageBuilder.getBytes();
        packet = new DatagramPacket(buf, buf.length, address, port);

        try {
            socket.send(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }

        socket.close();
    }

    public void close() {
        socket.close();
    }

    public static void main(String[] args) {
        new Server().run();
    }
}
