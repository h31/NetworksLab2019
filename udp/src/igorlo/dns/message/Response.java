package igorlo.dns.message;

public interface Response {

    byte[] getRawResponse();

    int getResponseType();

    int getResponseClass();

    int getTTL();

}
