package igorlo.dns;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public final class Utilities {

    //Do not use
    private Utilities() {
    }

    public static boolean isBitSet(final byte origin,
                                   final int index) {
        return ((origin << index) & 0b10000000) == 0b10000000;
    }

    public static byte getBitsFromTo(final byte origin,
                                     final int from,
                                     final int to) {
        byte buffer = origin;
        if (from > 0){
            buffer = (byte) (buffer << from);
            buffer = (byte) (buffer >> 1);
            buffer = (byte) (buffer & 0b01111111);
            buffer = (byte) (buffer >> (from - 1));
        }
        if (8 - to > 0){
            buffer = (byte) (buffer >> 1);
            buffer = (byte) (buffer & 0b01111111);
            buffer = (byte) (buffer >> 8 - to - 1);
        }
        return buffer;
    }

    public static int intFromTwoBytes(final byte left, final byte right){
        return ((left & 0xff) << 8) | (right & 0xff);
    }

    public static byte[] shortToBytes(final short origin){
        byte[] result = new byte[2];
        result[0] = (byte)(origin & 0xff);
        result[1] = (byte)((origin >> 8) & 0xff);
        return result;
    }

}
