package igorlo.dns;

import igorlo.dns.message.DnsMessage;
import igorlo.dns.message.DnsMessageBuilder;
import igorlo.dns.message.DnsMessageClass;

public class DnsClient {

    public static void main(String[] args) {
        DnsMessage message = new DnsMessageBuilder()
                .setId((short) 42)
                .addRequestTo("google.com")
                .build();

        System.out.println(message.toString());
    }

}

