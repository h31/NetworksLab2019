package dnsPackage.utilits;

import dnsPackage.parts.Answer;
import dnsPackage.parts.Header;
import dnsPackage.parts.Query;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class PackageReader {
    private Header header;
    private List<Query> query;
    private List<Answer> answer;

    public PackageReader() {
    }

    public void read(byte[] bytes) {
        int position = 0;
        position += readHeader(bytes);
        if (header.getQdCount() > 0) {
            position += readQuery(Arrays.copyOfRange(bytes, position, bytes.length));
        }
        if (header.getAdCount() > 0) {
            position += readAnswer(Arrays.copyOfRange(bytes, position, bytes.length));
        }

    }

    public int readHeader(byte[] bytes) {
        int position = 0;
        this.header = new Header(Arrays.copyOfRange(bytes, position, position + 12));
        return position + 12;
    }

    public int readQuery(byte[] bytes) {
        query = new ArrayList<>();
        int start = 0, end;
        for (int i = 0; i < header.getQdCount(); i++) {
            end = start;
            while (bytes[end] != 0) end++;
            end += 5;
            query.add(new Query(Arrays.copyOfRange(bytes, start, end)));
            start = end;
        }
        return start;
    }

    public int readAnswer(byte[] bytes) {
        answer = new ArrayList<>();
        int start = 0, end;
        for (int i = 0; i < header.getAdCount(); i++) {
            end = start + 12 + Utils.getIntFromTwoBytes(bytes[start + 10], bytes[start + 11]);
            answer.add(new Answer(Arrays.copyOfRange(bytes, start, end)));
            start = end;
        }
        return start;
    }

    public String getAnswerData() {
        StringBuilder stringBuilder = new StringBuilder();
        for (Answer a: answer) {
            stringBuilder.append(a.getData()).append("\n");
        }
        return stringBuilder.toString();
    }

    public Header getHeader() {
        return header;
    }

    public List<Query> getQuery() {
        return query;
    }

    public Answer getAnswer() {
        return null;
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("PackageRead: \n" + header.toString() + "\n");
        if (header.getQdCount() > 0) {
            for (Query q : query) stringBuilder.append(q.toString() + "\n");
        }
        if (header.getAdCount() > 0) {
            for (Answer a : answer) stringBuilder.append(a.toString() + "\n");
        }
        return stringBuilder.toString();
    }
}
