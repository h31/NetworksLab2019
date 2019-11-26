package igorlo.dns.message;

import java.util.Arrays;
import java.util.Collection;

public class DnsMessageClass implements DnsMessage {

    private final byte[] fullMessage;
    private final DnsFlags flags;

    public DnsMessageClass(byte[] rawMessage) {
        if (rawMessage == null) {
            throw new NullPointerException("Невозможно создать сообщение из NULL");
        }

        fullMessage = Arrays.copyOf(rawMessage, rawMessage.length);

        byte[] flagsBytes = Arrays.copyOfRange(rawMessage, 2, 4);
        flags = new DnsFlagsClass(flagsBytes);
    }

    @Override
    public byte[] getRawMessage() {
        return fullMessage;
    }

    @Override
    public int getId() {
        return ((fullMessage[0] & 0xff) << 8) | (fullMessage[1] & 0xff);
    }

    @Override
    public DnsFlags getFlags() {
        return flags;
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
    public String toString() {
        return "DnsMessageClass {\n" +
                "FullRaw = " + Arrays.toString(fullMessage) +
                "ID = " + getId() +
                "\nflags = " + flags.toString() +
                "\n}";
    }
}
