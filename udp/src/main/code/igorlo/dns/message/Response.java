package igorlo.dns.message;

import java.util.Arrays;

public class Response {

    private final String address;
    private final int type;
    private final int qClass;
    private final int ttl;
    private final int rLength;
    private final byte[] rData;

    public Response(String address, int type, int qClass, int ttl, int rLength, byte[] rData) {
        this.address = address;
        this.type = type;
        this.qClass = qClass;
        this.ttl = ttl;
        this.rLength = rLength;
        this.rData = rData;
    }

    public String getAddress() {
        return address;
    }

    public int getType() {
        return type;
    }

    public int getQClass() {
        return qClass;
    }

    public int getTtl() {
        return ttl;
    }

    public int getRLength() {
        return rLength;
    }

    public byte[] getRData() {
        return rData;
    }

    public byte[] buildBytes() {
        throw new UnsupportedOperationException("lol kek"); //TODO
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("[Response: Name = ");
        stringBuilder.append(getAddress());
        stringBuilder.append(", rType = ");
        stringBuilder.append(getType());
        stringBuilder.append(", rClass = ");
        stringBuilder.append(getQClass());
        stringBuilder.append(", rLength = ");
        stringBuilder.append(getRLength());
        stringBuilder.append(", rData = ");
        if (getType() == DnsType.A.getType()) {
            byte[] ipAddress = Arrays.copyOfRange(getRData(), 0, 4);
            stringBuilder
                    .append("IP[")
                    .append(ipAddress[0] & 0xff)
                    .append(".")
                    .append(ipAddress[1] & 0xff)
                    .append(".")
                    .append(ipAddress[2] & 0xff)
                    .append(".")
                    .append(ipAddress[3] & 0xff)
                    .append("]");
        } else {
            stringBuilder.append(Arrays.toString(getRData()));
        }
        return stringBuilder.toString();
    }
}
