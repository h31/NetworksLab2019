package igorlo.dns.message;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class MessageUtilsTest {

    @Test
    void intFromTwoBytes() {
        byte left, right;
        int result;

        left = 0b00000001;
        right = 0b00000001;
        result = MessageUtils.intFromTwoBytes(left, right);
        assert result == 257;

        left = 0b00000001;
        right = 0b00000000;
        result = MessageUtils.intFromTwoBytes(left, right);
        assert result == 256;

        left = 0b00000000;
        right = 0b00000001;
        result = MessageUtils.intFromTwoBytes(left, right);
        assert result == 1;

        left = 0b00000010;
        right = 0b00000010;
        result = MessageUtils.intFromTwoBytes(left, right);
        assert result == 514;

        left = 0b00000010;
        right = 0b00000010;
        result = MessageUtils.intFromTwoBytes(left, right);
        assert result == 514;
    }

    @org.junit.jupiter.api.Test
    void isBitSet() {
        final byte fifth = 0b00000100;
        final byte zero = (byte) 0b10000000;
        final byte first = 0b01000000;
        final byte seventh = 0b00000001;

        assert MessageUtils.isBitSet(fifth, 5);
        assert !MessageUtils.isBitSet(fifth, 4);
        assert !MessageUtils.isBitSet(fifth, 6);

        assert MessageUtils.isBitSet(zero, 0);
        assert !MessageUtils.isBitSet(zero, 1);
        assert !MessageUtils.isBitSet(zero, 5);
        assert !MessageUtils.isBitSet(zero, 7);

        assert MessageUtils.isBitSet(first, 1);
        assert !MessageUtils.isBitSet(first, 2);
        assert !MessageUtils.isBitSet(first, 3);
        assert !MessageUtils.isBitSet(first, 4);
        assert !MessageUtils.isBitSet(first, 5);
        assert !MessageUtils.isBitSet(first, 6);
        assert !MessageUtils.isBitSet(first, 7);
        assert !MessageUtils.isBitSet(first, 0);

        assert MessageUtils.isBitSet(seventh, 7);
        assert !MessageUtils.isBitSet(seventh, 6);
    }

    @Test
    void getBitsFromTo() {
        final byte first = 0b00010100;
        final byte second = (byte) 0b11010010;
        final byte third = 0b01011010;
        final byte fourth = (byte) 0b10000001;

        assert MessageUtils.getBitsFromTo(first, 3, 6) == 0b00000101;
        assert MessageUtils.getBitsFromTo(first, 0, 4) == 0b00000001;

        assert MessageUtils.getBitsFromTo(second, 0, 4) == 0b00001101;
        assert MessageUtils.getBitsFromTo(second, 5, 8) == 0b00000010;
        assert MessageUtils.getBitsFromTo(second, 2, 6) == 0b00000100;

        assert MessageUtils.getBitsFromTo(third, 4, 6) == 0b00000010;

        assert MessageUtils.getBitsFromTo(first, 0, 8) == first;
        assert MessageUtils.getBitsFromTo(second, 0, 8) == second;
        assert MessageUtils.getBitsFromTo(third, 0, 8) == third;
        assert MessageUtils.getBitsFromTo(fourth, 0, 8) == fourth;
    }

}