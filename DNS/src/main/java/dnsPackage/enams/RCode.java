package dnsPackage.enams;

public enum RCode {
    NO_ERR(0), //ошибки нет
    SERVER_ERR(1), //ошибка формата
    WRONG_NAME(2), //ошибка имени
    NO_IMPL(3), //отсутствует реализация
    DENIED(4); //отказано

    private int code;

    RCode(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static RCode getRCode(int code) {
        switch (code) {
            case 0:
                return RCode.NO_ERR;
            case 1:
                return RCode.SERVER_ERR;
            case 2:
                return RCode.WRONG_NAME;
            case 3:
                return RCode.NO_IMPL;
            case 4:
                return RCode.DENIED;
            default:
                return RCode.NO_ERR;
        }
    }
}
