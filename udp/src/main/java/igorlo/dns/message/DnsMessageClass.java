package igorlo.dns.message;

import kotlin.Pair;
import kotlin.text.Charsets;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Optional;

import static igorlo.dns.message.MessageUtils.intFromTwoBytes;
import static igorlo.dns.message.MessageUtils.shortFromTwoBytes;

public class DnsMessageClass implements DnsMessage {

    private static int ADDRESS_OFFSET = 192;

    private final byte[] fullMessage;
    private final DnsFlags flags;
    private Collection<Request> requests = null;
    private Collection<Response> responses = null;
    private Collection<Response> authorityResponses = null;
    private Collection<Response> additionalResponses = null;
    private Integer responsePointer = null;
    private Integer authorizedPointer = null;
    private Integer additionalPointer = null;

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
    public int getAuthorizedQuantity() {
        return intFromTwoBytes(fullMessage[8], fullMessage[9]);
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
            Pair<String, Integer> namePair = getDomainNameFromPointer(pointer);
            pointer += namePair.component2();
            String currentName = namePair.component1();
            short requestType = shortFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            short requestClass = shortFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            requests.add(new Request(currentName, requestType, requestClass));
        }
        responsePointer = pointer;
        return requests;
    }

    private Pair<String, Integer> getDomainNameFromPointer(int fromPointer) {
        int bytesRead = 0;
        StringBuilder currentName = new StringBuilder();
        while (true) {
            if (
                    MessageUtils.isBitSet(fullMessage[fromPointer + bytesRead], 0)
                            && MessageUtils.isBitSet(fullMessage[fromPointer + bytesRead], 1)
            ) {
//                currentName.append(getDomainNameFromPointer(
//                        intFromTwoBytes((byte) (fullMessage[fromPointer] - ADDRESS_OFFSET), fullMessage[fromPointer + 1])
//                ).component1());
                currentName.append(getDomainNameFromPointer(
                        MessageUtils.intFromSignedByte(fullMessage[fromPointer + bytesRead + 1])
                ).component1());
                bytesRead += 2;
                return new Pair<>(currentName.toString(), bytesRead);
            } else {
                Optional<Pair<String, Integer>> pair = readSegment(fromPointer + bytesRead);
                if (!pair.isPresent()) {
                    bytesRead += 1;
                    break;
                } else {
                    currentName.append(pair.get().component1());
                    bytesRead += pair.get().component2();
                }
            }
        }
        return new Pair<>(currentName.toString(), bytesRead);
    }

    private Optional<Pair<String, Integer>> readSegment(int fromPointer) {
        if (fullMessage[fromPointer] == 0) {
            return Optional.empty();
        }
        StringBuilder segmentBuilder = new StringBuilder();
        int numberOfSymbols = fullMessage[fromPointer];
        byte[] byteChunk = new byte[numberOfSymbols];
        for (int i = 0; i < numberOfSymbols; i++) {
            byteChunk[i] = fullMessage[fromPointer + i + 1];
        }
        segmentBuilder.append(new String(byteChunk, Charsets.US_ASCII));
        segmentBuilder.append('.');
        return Optional.of(new Pair<>(segmentBuilder.toString(), numberOfSymbols + 1));
    }

    @Override
    public Collection<Response> getResponses() {
        if (responses != null) {
            return responses;
        }
        responses = new ArrayList();
        if (responsePointer == null) {
            getRequests();
        }
        int pointer = responsePointer;
        for (int i = 0; i < getResponseQuantity(); i++) {
            Pair<String, Integer> namePair = getDomainNameFromPointer(pointer);
            pointer += namePair.component2();
            String currentName = namePair.component1();
            int responseType = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int responseClass = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int ttl = ByteBuffer.wrap(Arrays.copyOfRange(fullMessage, pointer, pointer + 4)).getInt(0);
            pointer += 4;
            int rLength = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            byte[] rData = Arrays.copyOfRange(fullMessage, pointer, pointer + rLength);
            pointer += rLength;
            responses.add(new Response(currentName, responseType, responseClass, ttl, rLength, rData));
        }
        authorizedPointer = pointer;
        return responses;
    }

    @Override
    public Collection<Response> getAuthorized() {
        if (authorityResponses != null) {
            return authorityResponses;
        }
        authorityResponses = new ArrayList();
        if (authorizedPointer == null) {
            getResponses();
        }
        int pointer = authorizedPointer;
        for (int i = 0; i < getAuthorizedQuantity(); i++) {
            Pair<String, Integer> namePair = getDomainNameFromPointer(pointer);
            pointer += namePair.component2();
            String currentName = namePair.component1();
            int responseType = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int responseClass = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int ttl = ByteBuffer.wrap(Arrays.copyOfRange(fullMessage, pointer, pointer + 4)).getInt(0);
            pointer += 4;
            int rLength = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            byte[] rData = Arrays.copyOfRange(fullMessage, pointer, pointer + rLength);
            pointer += rLength;
            authorityResponses.add(new Response(currentName, responseType, responseClass, ttl, rLength, rData));
        }
        additionalPointer = pointer;
        return authorityResponses;
    }

    @Override
    public Collection<Response> getAdditionalResponses() {
        if (additionalResponses != null) {
            return additionalResponses;
        }
        additionalResponses = new ArrayList();
        if (additionalPointer == null) {
            getAuthorized();
        }
        int pointer = additionalPointer;
        for (int i = 0; i < getAdditionalInfoQuantity(); i++) {
            Pair<String, Integer> namePair = getDomainNameFromPointer(pointer);
            pointer += namePair.component2();
            String currentName = namePair.component1();
            int responseType = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int responseClass = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            int ttl = ByteBuffer.wrap(Arrays.copyOfRange(fullMessage, pointer, pointer + 4)).getInt(0);
            pointer += 4;
            int rLength = intFromTwoBytes(fullMessage[pointer++], fullMessage[pointer++]);
            byte[] rData = Arrays.copyOfRange(fullMessage, pointer, pointer + rLength);
            pointer += rLength;
            additionalResponses.add(new Response(currentName, responseType, responseClass, ttl, rLength, rData));
        }
        return additionalResponses;
    }

    @Override
    public String toString() {
        return "DnsMessageClass {\n" +
                "FullRaw = " + Arrays.toString(fullMessage) +
                "\nID = " + getId() +
                "\nflags = " + flags.toString() +
                "\nRequests = " + getRequestsAsString() +
                "\nResponses = " + getResponsesAsString(getResponses()) +
                "\nAuthResponses = " + getResponsesAsString(getAuthorized()) +
                "\nAdditionalResponses = " + getResponsesAsString(getAdditionalResponses()) +
                "\n}";
    }

    private String getResponsesAsString(Collection<Response> responses) {
        StringBuilder stringBuilder = new StringBuilder();
        for (Response r : responses) {
            stringBuilder.append(r.toString());
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
