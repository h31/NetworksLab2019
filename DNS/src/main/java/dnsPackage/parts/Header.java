package dnsPackage.parts;

import dnsPackage.utilits.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class Header {
    private int id; //идентификатор
    private Flags flags; //флаги
    private int qdCount; //число RR-записей в разделе вопроса
    private int anCount; //число RR-записей в разделе ответа
    private int nsCount; //число RR-записей серверов имен в разделе авторитета
    private int arCount; //число RR-записей в разделе дополнительных записей

    public Header() {

    }

    public Header(Flags flags) {
        setRandId();
        this.flags = flags;
        this.qdCount = 0;
        this.anCount = 0;
        this.nsCount = 0;
        this.arCount = 0;
    }

    public Header(byte[] bytes) {
        int position = 0;
        this.id = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.flags = new Flags(new byte[]{bytes[position++], bytes[position++]});
        this.qdCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.anCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.nsCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position++]);
        this.arCount = Utils.getIntFromTwoBytes(bytes[position++], bytes[position]);
    }

    public Header setDefQueryFlags() {
        setRandId();
        this.flags = new Flags().getDefaultQueryFlags();
        return this;
    }

    public List<Byte> getBytesList() {
        List<Byte> headerBytes = new ArrayList<>();
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(id)));
        headerBytes.addAll(flags.getBytesList());
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(qdCount)));
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(anCount)));
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(nsCount)));
        headerBytes.addAll(Utils.getByteList(Utils.getTwoBytesFromInt(arCount)));
        return headerBytes;
    }

    private void setRandId() {
        Random random = new Random();
        this.id = random.nextInt(Integer.parseInt("FFFF", 16));
    }

    public Flags getFlags() {
        return flags;
    }

    public int getQdCount() {
        return qdCount;
    }

    public int getAnCount() {
        return anCount;
    }

    public int getNsCount() {
        return nsCount;
    }

    public int getArCount() {
        return arCount;
    }

    public void setFlags(Flags flags) {
        this.flags = flags;
    }

    public void setQdCount(int qdCount) {
        this.qdCount = qdCount;
    }

    public void setAnCount(int anCount) {
        this.anCount = anCount;
    }

    public void setNsCount(int nsCount) {
        this.nsCount = nsCount;
    }

    public void setArCount(int arCount) {
        this.arCount = arCount;
    }

    @Override
    public String toString() {
        return "Header:" + "\n" +
                "id " + id + "\n" +
                flags.toString() + "\n" +
                "QdCount " + qdCount + "\n" +
                "AdCount " + anCount + "\n" +
                "NsCount " + nsCount + "\n" +
                "ArCount " + arCount;
    }
}
