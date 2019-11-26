package igorlo.dns.message;

import java.util.Arrays;
import java.util.Collection;

import static igorlo.dns.Utilities.intFromTwoBytes;

public class DnsMessageClass implements DnsMessage {

    private final byte[] fullMessage;
    private final DnsFlags flags;

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
        throw new UnsupportedOperationException("To be implemented");
    }

    @Override
    public Collection<Response> getResponses() {
        throw new UnsupportedOperationException("To be implemented");
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
                "\n}";
    }
}
