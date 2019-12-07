package dnsPackage.parts;

import dnsPackage.enams.OpCode;
import dnsPackage.enams.RCode;

import java.util.ArrayList;
import java.util.List;

public class Flags {
    private boolean qr; //0 - запрос, 1 - ответ
    private OpCode opCode; //тип запроса
    private boolean aa; //авторитетный ответ
    private boolean tc; //усечение (первышение максимально доступной длины)
    private boolean rd; //желательна рекурсия
    private boolean ra; //рекурсия доступна
    private RCode rCode; //код ошибки

    public Flags(){
    }

    public Flags(boolean qr, OpCode opCode, boolean aa, boolean tc, boolean rd, boolean ra, RCode rCode) {
        this.qr = qr;
        this.opCode = opCode;
        this.aa = aa;
        this.tc = tc;
        this.rd = rd;
        this.ra = ra;
        this.rCode = rCode;
    }

    public Flags(byte[] bytes) {
        this.qr = setFlagByBit(bytes[0], 7);
        this.opCode = OpCode.getOpCode(((bytes[0] >>> 3) & 0b1111));
        this.aa = setFlagByBit(bytes[0], 2);
        this.tc = setFlagByBit(bytes[0], 1);
        this.rd = setFlagByBit(bytes[0], 0);
        this.ra = setFlagByBit(bytes[1], 7);
        this.rCode = RCode.getRCode((bytes[1] & 0b1111));
    }

    public Flags getDefaultQueryFlags() {
        this.qr = false;
        this.opCode = OpCode.QUERY;
        this.aa = false;
        this.tc = false;
        this.rd = true;
        this.ra = false;
        this.rCode = RCode.NO_ERR;
        return this;
    }

    public List<Byte> getBytesList() {
        List<Byte> flagBytesList = new ArrayList<>();
        byte firstByte = 0;
        byte secondByte = 0;
        firstByte = addBit(firstByte, qr);
        firstByte <<= 4;
        firstByte += opCode.getCode();
        firstByte = addBit(firstByte, aa);
        firstByte = addBit(firstByte, tc);
        firstByte = addBit(firstByte, rd);

        secondByte = addBit(secondByte, ra);
        secondByte <<= 7;
        secondByte += rCode.getCode();

        flagBytesList.add(firstByte);
        flagBytesList.add(secondByte);
        return flagBytesList;
    }

    private byte addBit(byte b, boolean flag) {
        if (flag) {
            return (byte) ((b << 1) + 1);
        } else return (byte) (b << 1);
    }

    private boolean setFlagByBit(byte b, int digit) {
        return ((b & (byte) Math.pow(2, digit)) != 0);
    }

    public RCode getRCode() {
        return  this.rCode;
    }

    public void setRd(boolean rd) {
        this.rd = rd;
    }

    public Flags setRCode(RCode rCode) {
        this.rCode = rCode;
        return this;
    }

    @Override
    public String toString() {
        return  "QR: " + qr + "\n" +
                "OpCode: " + opCode + "\n" +
                "AA: " + aa + "\n" +
                "TC: " + tc + "\n" +
                "RD: " + rd + "\n" +
                "RA: " + ra + "\n" +
                "RCode: " + rCode;
    }
}
