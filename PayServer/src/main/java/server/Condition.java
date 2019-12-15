package server;

public enum Condition {
    NORMAL(0),
    WAITING_FOR_ACCEPT(1),
    ACCEPTING(2);

    private int code;

    Condition(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
