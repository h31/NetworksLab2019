package dnsPackage.enams;

public enum RRType {
    A(1), //address IPv4
    AAAA(28), //address IPv6
    NS(2), //name server
    NOT(0);

    private int code;

    RRType(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static RRType getRRType(int code) {
        for (RRType r: RRType.values()) {
            if(r.getCode() == code) return r;
        }
        return RRType.NOT;
    }
}
