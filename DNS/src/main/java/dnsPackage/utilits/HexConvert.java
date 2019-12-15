package dnsPackage.utilits;

public class HexConvert {
    public static byte hextoByte(String hex) {
        int digit1 = toDigit(hex.charAt(0));
        int digit2 = toDigit(hex.charAt(1));
        return  (byte) ((digit1 << 4) + digit2);
    }

    private static int toDigit(char hexChar) {
        int digit = Character.digit(hexChar, 16);
        if(digit == -1) {
            throw new IllegalArgumentException(
                    "Invalid Hexadecimal Character: "+ hexChar);
        }
        return digit;
    }

    public static String byteToHex(byte num) {
        char[] hexDigits = new char[2];
        hexDigits[0] = Character.forDigit((num >> 4) & 0xF, 16);
        hexDigits[1] = Character.forDigit((num & 0xF), 16);
        return new String(hexDigits);
    }


}
