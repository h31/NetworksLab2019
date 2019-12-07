package dnsPackage.utilits;

import dnsPackage.enams.RRType;
import dnsPackage.parts.Answer;
import dnsPackage.parts.Header;
import dnsPackage.parts.Query;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class PackageReader {
    static final int HEADER_LEN = 12;
    private Header header;
    private List<Query> queries;
    private List<Answer> answers;
    private List<Answer> authoritative;
    private List<Answer> additional;
    private byte[] bytes;

    public PackageReader() {
        queries = new ArrayList<>();
        answers = new ArrayList<>();
        authoritative = new ArrayList<>();
        additional = new ArrayList<>();
    }

    public void read(byte[] bytes) {
        this.bytes = bytes;
        int position = 0;
        position += readHeader(bytes);
        if (header.getQdCount() > 0) {
            position += readQuery(Arrays.copyOfRange(bytes, position, bytes.length));
        }
        if (header.getAnCount() > 0) {
            answers = readAnswer(bytes, position, header.getAnCount());
            position += getAnswersSize(answers);
        }
        if (header.getNsCount() > 0) {
            authoritative = readAnswer(bytes, position, header.getNsCount());
            position += getAnswersSize(authoritative);

        }
        if (header.getArCount() > 0) {
            additional = readAnswer(bytes, position, header.getArCount());
            position += getAnswersSize(additional);
        }
    }

    private int readHeader(byte[] bytes) {
        int position = 0;
        this.header = new Header(Arrays.copyOfRange(bytes, position, position + 12));
        return position + 12;
    }

    private int readQuery(byte[] bytes) {
        queries = new ArrayList<>();
        int start = 0, end;
        for (int i = 0; i < header.getQdCount(); i++) {
            end = start;
            while (bytes[end] != 0) end++;
            end += 5;
            queries.add(new Query(Arrays.copyOfRange(bytes, start, end)));
            start = end;
        }
        return start;
    }

    private List<Answer> readAnswer(byte[] bytes, int position, int number) {
        List<Answer> answers = new ArrayList<>();
        int end;
        for (int i = 0; i < number; i++) {
            end = position + HEADER_LEN + Utils.getIntFromTwoBytes(bytes[position + 10], bytes[position + 11]);
            Answer answer = new Answer(Arrays.copyOfRange(bytes, position, end));
            answer.setNameString(getStringName(position, bytes));
            if (answer.getRrType() == RRType.NS) answer.setrDataString(getStringName(position + 12, bytes));
            answers.add(answer);
            position = end;
        }
        return answers;
    }

    private int getAnswersSize(List<Answer> answers) {
        int size = 0;
        for (Answer a : answers) size += a.length();
        return size;
    }

    public String getAnswerData() {
        StringBuilder stringBuilder = new StringBuilder();
        for (Answer a : answers) {
            stringBuilder.append(a.getData()).append("\n");
        }
        return stringBuilder.toString();
    }

    private String getStringName(int position, byte[] bytes) {
        StringBuilder stringBuilder = new StringBuilder();
        while (bytes[position] != 0) {
            if (Utils.byteToUnsignedInt(bytes[position]) == 0xC0) {
                position = Utils.byteToUnsignedInt(bytes[position + 1]);
                continue;
            }
            int len = bytes[position];
            position++;
            for (int j = position; j < position + len; j++) stringBuilder.append((char) bytes[j]);
            stringBuilder.append(".");
            position += len;
        }
        if (stringBuilder.length() > 0) stringBuilder.setLength(stringBuilder.length() - 1);
        return stringBuilder.toString();
    }

    public Header getHeader() {
        return header;
    }

    public List<Query> getQueries() {
        return queries;
    }

    public List<Answer> getAnswers() {
        return answers;
    }

    public List<Answer> getAuthoritative() {
        return authoritative;
    }

    public List<Answer> getAdditional() {
        return additional;
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("PackageRead: \n").append(header.toString()).append("\n");
        if (header.getQdCount() > 0) {
            for (Query q : queries) stringBuilder.append(q.toString()).append("\n");
        }
        if (header.getAnCount() > 0) {
            stringBuilder.append("\nAnswers:\n");
            for (Answer a : answers) stringBuilder.append(a.toString()).append("\n");
        }
        if (header.getNsCount() > 0) {
            stringBuilder.append("\nAuthoritative:\n");
            for (Answer a : authoritative) stringBuilder.append(a.toString()).append("\n");
        }
        if (header.getArCount() > 0) {
            stringBuilder.append("\nAdditional:\n");
            for (Answer a : additional) stringBuilder.append(a.toString()).append("\n");
        }

        return stringBuilder.toString();
    }

}
