package igorlo.dns.message;

public interface Response {

    byte[] getRawResponse();

    String getResponseAdress();

    int getResponseType();

    int getResponseClass();

    int getTTL();

}
