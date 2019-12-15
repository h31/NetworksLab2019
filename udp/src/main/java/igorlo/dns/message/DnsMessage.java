package igorlo.dns.message;

import java.util.Collection;

public interface DnsMessage {

    byte[] getRawMessage();

    int getId();

    DnsFlags getFlags();

    int getRequestQuantity();

    int getResponseQuantity();

    int getAuthorizedQuantity();

    int getAdditionalInfoQuantity();

    Collection<Request> getRequests();

    Collection<Response> getResponses();

    Collection<Response> getAuthorized();

    Collection<Response> getAdditionalResponses();

}
