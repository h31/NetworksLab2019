package dnsPackage.parts;

import dnsPackage.utilits.Utils;

import javax.swing.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class Header {
    private int id; //идентификатор
    private Flags flags; //флаги
    private int qdCount; //число RR-записей в разделе вопроса
    private int adCount; //число RR-записей в разделе ответа
    private int nsCount; //число RR-записей серверов имен в разделе авторитета
    private int arCount; //число RR-записей в разделе дополнительных записей

    public Header() {
    }

    public Header(Flags flags) {
        Random random = new Random();
        this.id = random.nextInt(Integer.parseInt("FFFF", 16));
        this.flags = flags;
        this.qdCount = 0;
        this.adCount = 0;
        this.nsCount = 0;
        this.arCount = 0;
    }

    public Header setDefQueryFlags() {
        this.flags = new Flags().getDefaultQueryFlags();
        return this;
    }

    public Header(byte[] bytes) {
        int position = 0;
        this.id = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.flags = new Flags(new byte[]{bytes[position++], bytes[position++]});
        this.qdCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.adCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.nsCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.arCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position]);
    }

    public List<Byte> getBytesList() {
        List<Byte> headerBytes = new ArrayList<>();
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(id)));
        headerBytes.addAll(flags.getBytesList());
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(qdCount)));
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(adCount)));
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(nsCount)));
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(arCount)));
        return headerBytes;
    }

    public Flags getFlags() {
        return flags;
    }

    public int getQdCount() {
        return qdCount;
    }

    public int getAdCount() {
        return adCount;
    }

    public void setFlags(Flags flags) {
        this.flags = flags;
    }

    public void setQdCount(int qdCount) {
        this.qdCount = qdCount;
    }

    public void setAdCount(int adCount) {
        this.adCount = adCount;
    }

    @Override
    public String toString() {
        return "Header:" + "\n" +
                "id " + id + "\n" +
                flags.toString() + "\n" +
                "QdCount " + qdCount + "\n" +
                "AdCount " + adCount + "\n" +
                "NsCount " + nsCount + "\n" +
                "ArCount " + arCount;
    }
}
