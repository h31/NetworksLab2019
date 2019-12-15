package dnsPackage.enams;

public enum RCode {
    NO_ERR(0), //ошибки нет
    FORMAT_ERR(1), //ошибка формата
    SERVER_ERR(2), //ошибка сервера
    WRONG_NAME(3), //ошибка имени
    NO_IMPL(4), //отсутствует реализация
    DENIED(5); //отказано

    private int code;

    RCode(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }

    public static RCode getRCode(int code) {
        for (RCode r: RCode.values()) {
            if(r.getCode() == code) return r;
        }
        return RCode.NO_ERR;
    }
}
