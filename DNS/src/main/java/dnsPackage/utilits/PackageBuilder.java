package dnsPackage.utilits;

import dnsPackage.enams.RCode;
import dnsPackage.parts.Answer;
import dnsPackage.parts.Flags;
import dnsPackage.parts.Header;
import dnsPackage.parts.Query;

import java.util.ArrayList;
import java.util.List;

public class PackageBuilder {
    private Header header;
    private List<Query> query;
    private List<Answer> answer;
    private List<Answer> authoritative;
    private List<Answer> additional;
    byte[] bytes;

    public PackageBuilder(){
        query = new ArrayList<>();
        answer = new ArrayList<>();
        authoritative = new ArrayList<>();
        additional = new ArrayList<>();
    }

    public PackageBuilder addHeader(Header header) {
        this.header = header;
        return this;
    }

    public PackageBuilder addQuery(List<Query> query) {
        this.query.addAll(query);
        return this;
    }

    public PackageBuilder addQuery(Query query) {
        this.query.add(query);
        return this;
    }

    public PackageBuilder addAnswer(List<Answer> answer) {
        this.answer.addAll(answer);
        return this;
    }

    public  PackageBuilder addAnswer(Answer answer) {
        this.answer.add(answer);
        return this;
    }

    public PackageBuilder addAuthoritative(List<Answer> answer) {
        this.authoritative.addAll(answer);
        return this;
    }

    public PackageBuilder addAdditional(List<Answer> answer) {
        this.additional.addAll(answer);
        return this;
    }

    public PackageBuilder build() {
        header.setQdCount(query.size());
        header.setAnCount(answer.size());
        header.setNsCount(authoritative.size());
        header.setArCount(additional.size());
        List<Byte> queryPackageList = new ArrayList<>();
        queryPackageList.addAll(header.getBytesList());
        for (Query q: query) {
            queryPackageList.addAll(q.getBytesList());
        }
        for (Answer a: answer) queryPackageList.addAll(a.getBytesList());
        this.bytes = Utils.getByteArray(queryPackageList);
        return this;
    }

    public byte[] getBytes() {
        return this.bytes;
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("Packageuild: \n" + header.toString() + "\n");
        if (header.getQdCount() > 0) {
            for (Query q : query) stringBuilder.append(q.toString() + "\n");
        }
        if (header.getAnCount() > 0) {
            for (Answer a : answer) stringBuilder.append(a.toString() + "\n");
        }
        return stringBuilder.toString();
    }
}
