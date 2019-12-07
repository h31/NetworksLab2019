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

    private String nameString;
    private String rDataString;

    public Answer() {

    }

    public Answer(byte[] bytes) {
        int position = 0;
        this.name = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.rrType = RRType.getRRType(Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]));
        this.rrClass = RRClass.getRRClass(Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]));
        this.ttl = Utils.getIntFromFourBytes(bytes[position++], bytes[position++], bytes[position++], bytes[position++]);
        this.rdLength = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        rData = new int[bytes.length - position];
        for (int i = 0; i < rData.length; i++) {
            rData[i] = Utils.byteToUnsignedInt(bytes[position++]);
        }
        rDataString = "";
        nameString = "";
    }

    public Answer setName(int shift) {
        int mask = 0b1100000000000000;
        this.name = shift | mask;
        return this;
    }

    public Answer setRRType(RRType rrType) {
        this.rrType = rrType;
        return this;
    }

    public Answer setRRClass(RRClass rrClass) {
        this.rrClass = rrClass;
        return this;
    }

    public Answer setTtl(int ttl) {
        this.ttl = ttl;
        return this;
    }

    public Answer makeAnswer(String data) {
        switch (rrType) {
            case A:
                setATypeData(data);
        }
        return this;
    }

    private void setATypeData(String data) {
        String[] address = data.split("\\.");
        rData = new int[4];
        int i = 0;
        for (String s : address) {
            rData[i++] = Integer.parseInt(s);
        }
        this.rdLength = rData.length;
    }

    public String getATypeData() {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i: rData) stringBuilder.append(i).append(".");
        stringBuilder.setLength(stringBuilder.length() - 1);
        return stringBuilder.toString();
    }

    public List<Byte> getBytesList() {
        List<Byte> answerBytesList = new ArrayList<>();
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(name)));
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rrType.getCode())));
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rrClass.getCode())));
        answerBytesList.addAll(Utils.getByteList(Utils.getFourBytesFromInt(ttl)));
        answerBytesList.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(rdLength)));
        for (int i : rData) {
            answerBytesList.add((byte) i);
        }
        return answerBytesList;
    }

    public String getData() {
        StringBuilder stringBuilder = new StringBuilder();
        switch (rrType) {
            case A:
                for (int i : rData) stringBuilder.append(i).append(".");
                stringBuilder.setLength(stringBuilder.length() - 1);
                return stringBuilder.toString();
            case NS:
                for (int i : rData) stringBuilder.append((char) i);
                return stringBuilder.toString();
        }
        return "Do not support this RRType";
    }

    public int length() {
        return rData.length + 12;
    }

    public RRType getRrType() {
        return rrType;
    }

    public int getRdLength() {
        return rdLength;
    }

    public String getNameString() {
        return nameString;
    }

    public void setNameString(String nameString) {
        this.nameString = nameString;
    }

    public String getrDataString() {
        return rDataString;
    }

    public void setrDataString(String rDataString) {
        this.rDataString = rDataString;
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("Answer: {" +
                "name: " + Integer.toHexString(name).toUpperCase() +
                "; stringName: " + nameString +
                "; rrType: " + rrType +
                "; rrClass: " + rrClass +
                "; ttl: " + ttl +
                "; rdLength: " + rdLength +
                "; rData: [");
        for (int i : rData) stringBuilder.append(Integer.toHexString(i).toUpperCase()).append(", ");
        stringBuilder.setLength(stringBuilder.length() - 2);
        stringBuilder.append("]; " +
                "; stringData: " + rDataString +
                "}");
        return stringBuilder.toString();
    }
}
