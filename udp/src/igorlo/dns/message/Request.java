package igorlo.dns.message;

public interface Request {

    byte[] getRawRequest();

    String getDomainName();

    int getRequestType();

    int getRequestClass();

}
