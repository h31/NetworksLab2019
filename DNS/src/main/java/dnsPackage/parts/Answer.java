package dnsPackage.parts;

import dnsPackage.enams.RRClass;
import dnsPackage.enams.RRType;
import dnsPackage.utilits.Utils;

import java.util.ArrayList;
import java.util.List;

public class Answer {
    private int name;
    private RRType rrType;
    private RRClass rrClass;
    private int ttl;
    private int rdLength;
    private int[] rData;

    public Answer() {
    }

    public void makeATypeAnswer(int shift, int ttl,  RRClass rrClass, String data) {
        int mask = 0b1100000000000000;
        this.name = shift | mask;
        this.rrType = RRType.A;
        this.rrClass = rrClass;
        this.ttl = ttl;
        this.rdLength = 4;
        String[] address = data.split("\\.");
        rData = new int[4];
        int i = 0;
        for (String s: address) {
            rData[i++] = Integer.parseInt(s);
        }
    }

    public List<Byte> getBytesList() {
        List<Byte> answerBytesList = new ArrayList<>();
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(name)));
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rrType.getCode())));
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rrClass.getCode())));
        answerBytesList.addAll(Utils.getByteList(Utils.getFourBytesFromInt(ttl)));
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rdLength)));
        for (int i: rData) {
            answerBytesList.add((byte) i);
        }
        return answerBytesList;
    }
}
