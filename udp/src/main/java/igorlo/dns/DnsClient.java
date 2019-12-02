package igorlo.dns;

import igorlo.dns.message.DnsMessage;
import igorlo.dns.message.DnsMessageClass;

public class DnsClient {

    public static void main(String[] args) {
        byte[] question = {2, 34, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 97, 97, 97, 0, 0, 1, 0, 1};
        DnsMessage message = new DnsMessageClass(question);
        System.out.println(message.toString());
    }

}

