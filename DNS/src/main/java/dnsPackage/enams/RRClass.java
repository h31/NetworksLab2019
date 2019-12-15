package dnsPackage.enams;

public enum RRClass {
    IN(1), //Internet
    NOT(0);

    private int code;

    RRClass(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static RRClass getRRClass(int code) {
        for (RRClass r: RRClass.values()){
            return r;
        }
        return RRClass.NOT;
    }
}
