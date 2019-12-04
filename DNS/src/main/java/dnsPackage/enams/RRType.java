package dnsPackage.enams;

public enum RRType {
    A(1); //address

    private int code;

    RRType(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
