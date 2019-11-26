package igorlo.dns.message;

import java.util.ArrayList;
import java.util.Collection;

import static igorlo.dns.Utilities.shortToBytes;
import static igorlo.dns.message.Constants.*;

public class DnsMessageBuilder {

    //TODO

    private short id = DEFAULT_ID;
    private DnsFlags flags = DnsFlagsClass.DEFAULT_QUERRY_FLAGS;
    private short numberOfRequests = 0;
    private short numberOfResponses = 0;
    private short numberOfAdditionalInfo = 0;
    private Collection<Request> requests;
    private Collection<Response> responses;

    public DnsMessage build(){
//        byte[] header = new byte[12];
//        byte[] convertedId = shortToBytes(id);
        throw new UnsupportedOperationException("To be implemented");
    }

    public void setId(short id) {
        this.id = id;
    }

    public void setFlags(DnsFlags flags) {
        this.flags = flags;
    }

    public void addRequest(Request request){
        if (requests == null){
            requests = new ArrayList<>();
        }
        numberOfRequests++;
        requests.add(request);
    }

    public void addResponse(Response response) {
        if (responses == null) {
            responses = new ArrayList<>();
        }
        numberOfResponses++;
        responses.add(response);
    }

}
