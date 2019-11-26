package igorlo.dns.message;

import static igorlo.dns.Utilities.getBitsFromTo;
import static igorlo.dns.Utilities.isBitSet;

public class DnsFlagsClass implements DnsFlags {

    private final byte first;
    private final byte second;

    public DnsFlagsClass(byte[] flagsBytes) {
        if (flagsBytes.length != 2){
            throw new IllegalArgumentException("Flags cannot have other than 2 bytes : found " + flagsBytes.length);
        }
        first = flagsBytes[0];
        second = flagsBytes[1];
    }

    @Override
    public boolean isRequest() {
        return !isBitSet(first, 0);
    }

    @Override
    public int getOperationCode() {
        byte result = first;
        result = (byte) ((result & 0b01110000) >> 4);
        return result;
    }

    @Override
    public boolean getAA() {
        return isBitSet(first, 5);
    }

    @Override
    public boolean getTC() {
        return isBitSet(first, 6);
    }

    @Override
    public boolean getRD() {
        return isBitSet(first, 7);
    }

    @Override
    public boolean getRA() {
        return isBitSet(second, 0);
    }

    @Override
    public int getResponseCode() {
        return getBitsFromTo(second, 4, 7);
    }

    @Override
    public String toString() {
        return "DnsFlagsClass {\n" +
                "isRequest = " + isRequest() +
                "\nOperationCode = " + getOperationCode() +
                "\nAA = " + getAA() +
                "\nTC = " + getTC() +
                "\nRD = " + getRD() +
                "\nRA = " + getRA() +
                "\nResponseCode = " + getResponseCode() +
                "\n}";
    }
}
