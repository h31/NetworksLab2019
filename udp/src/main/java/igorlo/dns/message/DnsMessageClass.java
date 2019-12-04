package igorlo.dns.message;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import static igorlo.dns.Utilities.intFromTwoBytes;
import static igorlo.dns.Utilities.shortFromTwoBytes;

public class DnsMessageClass implements DnsMessage {

    private final byte[] fullMessage;
    private final DnsFlags flags;
    private Collection<Request> requests = null;
    private Collection<Response> responses = null;
    private Integer responsesePointer = null;
    private Integer nameServersPointer = null;

    public DnsMessageClass(byte[] rawMessage) {
        if (rawMessage == null) {
            throw new NullPointerException("Невозможно создать сообщение из NULL");
        }
        fullMessage = Arrays.copyOf(rawMessage, rawMessage.length);
        flags = new DnsFlagsClass(rawMessage[2], rawMessage[3]);
    }

    @Override
    public byte[] getRawMessage() {
        return fullMessage;
    }

    @Override
    public int getId() {
        return intFromTwoBytes(fullMessage[0], fullMessage[1]);
    }

    @Override
    public DnsFlags getFlags() {
        return flags;
    }

    @Override
    public int getRequestQuantity() {
        return intFromTwoBytes(fullMessage[4], fullMessage[5]);
    }

    @Override
    public int getResponseQuantity() {
        return intFromTwoBytes(fullMessage[6], fullMessage[7]);
    }

    @Override
    public int getAdditionalInfoQuantity() {
        return intFromTwoBytes(fullMessage[10], fullMessage[11]);
    }

    @Override
    public Collection<Request> getRequests() {
        if (requests != null) {
            return requests;
        }
        int pointer = 12;
        requests = new ArrayList();
        for (int i = 0; i < getRequestQuantity(); i++) {
            String currentName = getDomainNameFromPointer(pointer);
            pointer += currentName.length() + 1;
            short requestType = shortFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            short requestClass = shortFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            requests.add(new Request(currentName, requestType, requestClass));
        }
        responsesePointer = pointer;
        return requests;
    }

    private String getDomainNameFromPointer(int fromPointer) {
        int pointer = fromPointer;
        StringBuilder currentName = new StringBuilder();
        byte numberOfSymbols = fullMessage[pointer++];
        while (numberOfSymbols != 0) {
            for (int i = 0; i < numberOfSymbols; i++) {
                currentName.append((char) fullMessage[pointer++]);
            }
            currentName.append('.');
            numberOfSymbols = fullMessage[pointer++];
        }
        return currentName.toString();
    }

    @Override
    public Collection<Response> getResponses() {
        if (responses != null) {
            return responses;
        }
        responses = new ArrayList();
        if (responsesePointer == null) {
            getRequests();
        }
        int pointer = responsesePointer;
        for (int i = 0; i < getResponseQuantity(); i++) {
            String currentName = getDomainNameFromPointer(
                    intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++])
            );
            int responseType = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int responseClass = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int ttl = ByteBuffer.wrap(Arrays.copyOfRange(fullMessage, pointer, pointer + 4)).getInt(0);
            pointer += 4;
            int rLength = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            byte[] rData = Arrays.copyOfRange(fullMessage, pointer, pointer + rLength);
            pointer += rLength;
            responses.add(new Response(currentName, responseType, responseClass, ttl, rLength, rData));
        }
        nameServersPointer = pointer;
        return responses;
    }

    @Override
    public DnsAdditionalInfo getAdditionalInfo() {
        return null;
    }

    @Override
    public String toString() {
        return "DnsMessageClass {\n" +
                "FullRaw = " + Arrays.toString(fullMessage) +
                "ID = " + getId() +
                "\nflags = " + flags.toString() +
                "\nRequests = " + getRequestsAsString() +
                "\nResponses = " + getResponsesAsString() +
                "\n}";
    }

    private String getResponsesAsString() {
        StringBuilder stringBuilder = new StringBuilder();
        for (Response r : getResponses()) {
            stringBuilder.append("[Response: Name = ");
            stringBuilder.append(r.getAddress());
            stringBuilder.append(", rType = ");
            stringBuilder.append(r.getType());
            stringBuilder.append(", rClass = ");
            stringBuilder.append(r.getQClass());
            stringBuilder.append(", rLength = ");
            stringBuilder.append(r.getRLength());
            stringBuilder.append(", rData = ");
            stringBuilder.append(Arrays.toString(r.getRData()));
            stringBuilder.append("]\n");
        }
        return stringBuilder.toString();
    }

    private String getRequestsAsString() {
        StringBuilder stringBuilder = new StringBuilder();
        for (Request r : getRequests()) {
            stringBuilder.append("[REQUEST: Name = ");
            stringBuilder.append(r.getRequestAddress());
            stringBuilder.append(", qType = ");
            stringBuilder.append(r.getRequestType());
            stringBuilder.append(", qClass = ");
            stringBuilder.append(r.getRequestClass());
            stringBuilder.append("]\n");
        }
        return stringBuilder.toString();
    }
}
