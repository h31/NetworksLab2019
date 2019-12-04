package dnsPackage.utilits;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public final class Utils {
    public static int getIntFromTwoBytes(byte byte1, byte byte2) {
        return (byteToUnsignedInt(byte1) << 8) +
                byteToUnsignedInt(byte2);
    }

    public static int getIntFromFourBytes(byte byte1, byte byte2, byte byte3, byte byte4) {
        return (byteToUnsignedInt(byte1) << 24) +
                (byteToUnsignedInt(byte2) << 16) +
                (byteToUnsignedInt(byte3) << 8) +
                byteToUnsignedInt(byte4);
    }

    public static Byte[] getTwoBytesFromInt(int a) {
        return new Byte[]{
                (byte) (a >>> 8),
                (byte) (a)
        };
    }

    public static Byte[] getFourBytesFromInt(int a) {
        return new Byte[]{
                (byte) (a >>> 24),
                (byte) (a >>> 16),
                (byte) (a >>> 8),
                (byte) (a)
        };
    }

    public static byte[] getByteArray(List<Byte> byteList) {
        byte[] byteArray = new byte[byteList.size()];
        for (int i = 0; i < byteArray.length; i++) {
            byteArray[i] = byteList.get(i);
        }
        return byteArray;
    }

    public static List<Byte> getByteList(Byte[] bytes) {
        return new ArrayList<>(Arrays.asList(bytes));
    }

    public static int byteToUnsignedInt(byte b) {
        if (b < 0) return b + 256;
        else return b;
    }
}
