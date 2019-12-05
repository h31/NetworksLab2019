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
        for(OpCode o: OpCode.values()){
            return o;
        }
        return OpCode.QUERY;
    }

}
