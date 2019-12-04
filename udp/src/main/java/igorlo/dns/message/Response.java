package igorlo.dns.message;

class Response {

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
}
