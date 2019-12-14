package igorlo.dns.message;

import java.util.ArrayList;
import java.util.List;

import static igorlo.dns.message.MessageUtils.shortToBytes;

public class Request {

    private final String address;
    private final short type;
    private final short rClass;

    public Request(String address, short type, short rClass) {
        this.address = address;
        this.type = type;
        this.rClass = rClass;
    }

    public Request(String address) {
        this(address, (short) 1, (short) 1);
    }

    String getRequestAddress() {
        return address;
    }

    short getRequestType() {
        return type;
    }

    short getRequestClass() {
        return rClass;
    }

    public byte[] buildBytes() {
        List<byte[]> bytes = new ArrayList<>();
        bytes.add(addressToBytes());
        bytes.add(shortToBytes(type));
        bytes.add(shortToBytes(rClass));
        return MessageUtils.convertListOfArraysToArrayOfBytes(bytes);
    }

    private byte[] addressToBytes() {
        String[] splited = address.split("\\.");
        List<Byte> bytes = new ArrayList<>();
        for (String part : splited) {
            bytes.add((byte) part.length());
            for (int ch : part.chars().toArray()) {
                bytes.add((byte) ch);
            }
        }
        bytes.add((byte) 0);
        return MessageUtils.convertListToByteArray(bytes);
    }

}
