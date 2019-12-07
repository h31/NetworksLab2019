import dnsPackage.enams.RCode;
import dnsPackage.enams.RRClass;
import dnsPackage.enams.RRType;
import dnsPackage.parts.Answer;
import dnsPackage.parts.Header;
import dnsPackage.parts.Query;
import dnsPackage.utilits.Utils;
import server.Server;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        int[] a = {192, 233};
        byte[] b = new byte[a.length];
        for (int i = 0; i < a.length; i++) b[i] = (byte) a[i];
        System.out.println(b[0]);
    }
}
