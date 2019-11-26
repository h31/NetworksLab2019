package igorlo.dns.message;

public interface Request {

    byte[] getRawRequest();

    String getRequestAdress();

    int getRequestType();

    int getRequestClass();

}
