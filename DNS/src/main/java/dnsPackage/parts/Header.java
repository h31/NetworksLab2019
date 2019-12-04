package dnsPackage.parts;

import dnsPackage.utilits.Utils;

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

    public Header(int id, Flags flags, int qdCount, int adCount, int nsCount, int arCount) {
        this.id = id;
        this.flags = flags;
        this.qdCount = qdCount;
        this.adCount = adCount;
        this.nsCount = nsCount;
        this.arCount = arCount;
    }

    public static Header getDefaultQueryHeader() {
        Random random = new Random();
        return new Header(random.nextInt(Integer.parseInt("FFFF", 16)),
                Flags.getDefaultQueryFlags(),
                1, 0, 0, 0);
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

    @Override
    public String toString() {
        return "Header{" +
                "id=" + id +
                ", flags=" + flags.toString() +
                ", qdCount=" + qdCount +
                ", adCount=" + adCount +
                ", nsCount=" + nsCount +
                ", arCount=" + arCount +
                '}';
    }
}
