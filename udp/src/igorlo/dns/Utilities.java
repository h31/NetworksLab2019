package igorlo.dns;

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

}
