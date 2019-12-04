package igorlo.dns.message;

import static igorlo.dns.Utilities.getBitsFromTo;
import static igorlo.dns.Utilities.isBitSet;

public class DnsFlagsClass implements DnsFlags {

    public static final DnsFlags DEFAULT_QUERRY_FLAGS = new DnsFlagsClass(
            (byte) 0b00000001, (byte) 0b00000000
    );
    public static final DnsFlags DEFAULT_RESPONSE_FLAGS = new DnsFlagsClass(
            (byte) 0b10000001, (byte) 0b10000000
    );

    private final byte first;
    private final byte second;

    public DnsFlagsClass(final byte first, final byte second){
        this.first = first;
        this.second = second;
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
    public byte getFirst() {
        return first;
    }

    @Override
    public byte getSecond() {
        return second;
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
