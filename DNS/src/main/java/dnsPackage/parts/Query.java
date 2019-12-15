package dnsPackage.parts;

import dnsPackage.enams.RRClass;
import dnsPackage.enams.RRType;
import dnsPackage.utilits.Utils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Query {
    private int[] qName;
    private RRType rrType;
    private RRClass rrClass;

    public Query(String domainName) {
        domainToQName(domainName);
        rrType = rrType.A;
        rrClass = rrClass.IN;
    }

    public Query(String domainName, RRType rrType, RRClass rrClass) {
        domainToQName(domainName);
        this.rrType = rrType;
        this.rrClass = rrClass;
    }

    public Query(byte[] bytes) {
        int position;
        qName = new int[bytes.length - 4];
        for (position = 0; position < bytes.length - 4; position++) {
            qName[position] = Utils.byteToUnsignedInt(bytes[position]);
        }
        rrType = RRType.getRRType(Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]));
        rrClass = RRClass.getRRClass(Utils.getIntFromTwoBytes(bytes[position++], bytes[position]));
    }

    private void domainToQName(String domainName) {
        String[] domainNameArray = domainName.split("\\.");
        int[] qName = new int[domainName.length() + 2];
        int position = 0;
        for (String s : domainNameArray) {
            qName[position] = s.length();
            position++;
            char[] part = s.toCharArray();
            for (char c : part) {
                qName[position] = c;
                position++;
            }
        }
        qName[position] = 0;
        this.qName = qName;
    }

    public String getDomainName() {
        StringBuilder domainName = new StringBuilder();
        int position = 0;
        while (position < qName.length - 1) {
            int len = qName[position];
            position++;
            for (int j = position; j < position + len; j++) domainName.append((char) qName[j]);
            domainName.append(".");
            position += len;
        }
        domainName.setLength(domainName.length() - 1);
        return domainName.toString();
    }

    public int length() {
        return qName.length + 4;
    }

    public RRType getRrType() {
        return rrType;
    }

    public RRClass getRrClass() {
        return rrClass;
    }

    public List<Byte> getBytesList() {
        List<Byte> querryBytesList = new ArrayList<>();
        for (int i : qName) {
            querryBytesList.add((byte) i);
        }
        querryBytesList.addAll(Arrays.asList(Utils.getTwoBytesFromInt(rrType.getCode())));
        querryBytesList.addAll(Arrays.asList(Utils.getTwoBytesFromInt(rrClass.getCode())));
        return querryBytesList;
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("Query: {" +
                "qName: [");
        for (int i : qName) stringBuilder.append(Integer.toHexString(i).toUpperCase()).append(", ");
        stringBuilder.setLength(stringBuilder.length() - 2);
        stringBuilder.append("]; rrType: " + rrType +
                "; rrClass: " + rrClass +
                '}');
        return stringBuilder.toString();
    }
}
