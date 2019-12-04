package dnsPackage.enams;

public enum OpCode {
    QUERY(0), //стандартный запрос
    IQUERY(1), //инверсный запрос
    STATUS(2); //запрос состояния сервера

    private int code;

    OpCode(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static OpCode getOpCode(int code) {
        switch (code) {
            case 0:
                return OpCode.QUERY;
            case 1:
                return OpCode.IQUERY;
            case 2:
                return OpCode.STATUS;
            default:
                return OpCode.QUERY;
        }
    }

}
