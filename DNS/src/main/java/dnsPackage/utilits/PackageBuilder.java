package dnsPackage.utilits;

import dnsPackage.parts.Header;
import dnsPackage.parts.Query;

import java.util.ArrayList;
import java.util.List;

public final class PackageBuilder {
    public static byte[] makePackage(Header header, Query query) {
        List<Byte> queryPackageList = new ArrayList<>();
        queryPackageList.addAll(header.getBytesList());
        queryPackageList.addAll(query.getBytesList());
        byte[] queryPackage = new byte[queryPackageList.size()];
        for (int i = 0; i < queryPackage.length; i++) {
            queryPackage[i] = queryPackageList.get(i);
        }
        return queryPackage;
    }
}
