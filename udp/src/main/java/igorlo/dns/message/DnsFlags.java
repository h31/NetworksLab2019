package igorlo.dns.message;

public interface DnsFlags {

    boolean isRequest();

    int getOperationCode();

    boolean getAA();

    boolean getTC();

    boolean getRD();

    boolean getRA();

    int getResponseCode();

}
