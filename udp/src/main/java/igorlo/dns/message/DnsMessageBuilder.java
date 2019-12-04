package igorlo.dns.message;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import static igorlo.dns.Utilities.convertListOfArraysToArrayOfBytes;
import static igorlo.dns.Utilities.shortToBytes;
import static igorlo.dns.message.Constants.*;

public class DnsMessageBuilder {

    //TODO

    private short id = DEFAULT_ID;
    private DnsFlags flags = DnsFlagsClass.DEFAULT_QUERRY_FLAGS;
    private short numberOfRequests = 0;
    private short numberOfResponses = 0;
    private short numberOfNameServers = 0;
    private short numberOfAdditionalInfo = 0;
    private Collection<Request> requests;
    private Collection<Response> responses;

    public DnsMessage build() {
        List<byte[]> bytes = new ArrayList<>();
        bytes.add(buildHeader());
        if (requests != null) {
            for (Request request : requests) {
                bytes.add(request.buildBytes());
            }
        }
        if (responses != null) {
            for (Response response : responses) {
                bytes.add(response.buildBytes());
            }
        }
        byte[] byteArray = convertListOfArraysToArrayOfBytes(bytes);
        return new DnsMessageClass(byteArray);
    }

    private byte[] buildHeader() {
        byte[] header = new byte[12];
        byte[] idBytes = shortToBytes(id);
        header[0] = idBytes[0];
        header[1] = idBytes[1];
        header[2] = flags.getFirst();
        header[3] = flags.getSecond();
        byte[] nOfRequestsBytes = shortToBytes(numberOfRequests);
        header[4] = nOfRequestsBytes[0];
        header[5] = nOfRequestsBytes[1];
        byte[] nOfResponsesBytes = shortToBytes(numberOfResponses);
        header[6] = nOfResponsesBytes[0];
        header[7] = nOfResponsesBytes[1];
        byte[] nOfNameServersBytes = shortToBytes(numberOfNameServers);
        header[8] = nOfNameServersBytes[0];
        header[9] = nOfNameServersBytes[1];
        byte[] nOfAdditionalInfoBytes = shortToBytes(numberOfAdditionalInfo);
        header[10] = nOfAdditionalInfoBytes[0];
        header[11] = nOfAdditionalInfoBytes[1];
        return header;
    }

    public DnsMessageBuilder setId(short id) {
        this.id = id;
        return this;
    }

    public DnsMessageBuilder setFlags(DnsFlags flags) {
        this.flags = flags;
        return this;
    }

    public DnsMessageBuilder addRequest(Request request) {
        if (requests == null) {
            requests = new ArrayList<>();
        }
        numberOfRequests++;
        requests.add(request);
        return this;
    }

    public DnsMessageBuilder addRequestTo(String toWhere) {
        return addRequest(new Request(toWhere));
    }

    public DnsMessageBuilder addResponse(Response response) {
        if (responses == null) {
            responses = new ArrayList<>();
        }
        numberOfResponses++;
        responses.add(response);
        return this;
    }

}
