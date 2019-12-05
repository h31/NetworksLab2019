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

    public static RRType getRRType(int code) {
        switch (code) {
            case 1:
                return RRType.A;
            default:
                return RRType.A;
        }
    }
}
