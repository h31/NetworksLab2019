package server;

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

        //make dns query string
        /*buf = packet.getData();
        StringBuilder queryPackage = new StringBuilder();
        for (byte b : buf) {
            queryPackage.append(HexConvert.byteToHex(b)).append(" ");
        }*/


        String received
                = new String(packet.getData(), 0, packet.getLength());
        System.out.println("package received");

        InetAddress address = packet.getAddress();
        int port = packet.getPort();
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
