package dnsPackage.enams;

public enum RRClass {
    IN(1); //Internet

    private int code;

    RRClass(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
