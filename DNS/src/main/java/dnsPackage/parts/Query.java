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
        while (position < qName.length - 1){
            int len = qName[position];
            position++;
            for (int j = position; j < position + len; j++) domainName.append((char) qName[j]);
            domainName.append(".");
            position += len;
        }
        return domainName.toString();
    }

    public List<Byte> getBytesList() {
        List<Byte> querryBytesList = new ArrayList<>();
        for (int i : qName) {
            querryBytesList.add((byte) i);
        }
        querryBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rrType.getCode())));
        querryBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rrClass.getCode())));
        return querryBytesList;
    }

    @Override
    public String toString() {
        return "Query{" +
                "qName=" + Arrays.toString(qName) +
                ", rrType=" + rrType +
                ", rrClass=" + rrClass +
                '}';
    }
}
