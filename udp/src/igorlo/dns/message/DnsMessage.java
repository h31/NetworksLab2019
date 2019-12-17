package igorlo.dns.message;

import java.util.Collection;

public interface DnsMessage {

    byte[] getRawMessage();

    int getId();

    DnsFlags getFlags();

    Collection<Request> getRequests();

    Collection<Response> getResponses();

}
