package igorlo.dns.message;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

public final class MessageUtils {

    //Do not use
    private MessageUtils() {
    }

    public static boolean isBitSet(final byte origin,
                                   final int index) {
        return ((origin << index) & 0b10000000) == 0b10000000;
    }

    public static byte getBitsFromTo(final byte origin,
                                     final int from,
                                     final int to) {
        byte buffer = origin;
        if (from > 0) {
            buffer = (byte) (buffer << from);
            buffer = (byte) (buffer >> 1);
            buffer = (byte) (buffer & 0b01111111);
            buffer = (byte) (buffer >> (from - 1));
        }
        if (8 - to > 0) {
            buffer = (byte) (buffer >> 1);
            buffer = (byte) (buffer & 0b01111111);
            buffer = (byte) (buffer >> 8 - to - 1);
        }
        return buffer;
    }

    public static int intFromTwoBytes(final byte left, final byte right) {
        return ((left & 0xff) << 8) | (right & 0xff);
    }

    public static short shortFromTwoBytes(final byte left, final byte right) {
        return (short) (((left & 0xff) << 8) | (right & 0xff));
    }

    public static byte[] shortToBytes(final short origin) {
        byte[] result = new byte[2];
        result[1] = (byte) (origin & 0xff);
        result[0] = (byte) ((origin >> 8) & 0xff);
        return result;
    }

    public static byte[] intToBytes(final int origin) {
        byte[] result = new byte[4];
        result[3] = (byte) (origin & 0xff);
        result[2] = (byte) ((origin >> 8) & 0xff);
        result[1] = (byte) ((origin >> 16) & 0xff);
        result[0] = (byte) ((origin >> 24) & 0xff);
        return result;
    }

    public static byte[] convertListToByteArray(List<Byte> bytes) {
        byte[] array = new byte[bytes.size()];
        for (int i = 0; i < bytes.size(); i++) {
            array[i] = bytes.get(i);
        }
        return array;
    }

    public static byte[] convertListOfArraysToArrayOfBytes(List<byte[]> bytes) {
        List<Byte> unwrapped = new ArrayList<>();
        for (byte[] array: bytes) {
            for (byte b: array) {
                unwrapped.add(b);
            }
        }
        return convertListToByteArray(unwrapped);
    }
}
