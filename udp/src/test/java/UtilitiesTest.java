import igorlo.dns.Utilities;
import org.junit.jupiter.api.Test;

class UtilitiesTest {

    @org.junit.jupiter.api.Test
    void isBitSet() {
        final byte fifth = 0b00000100;
        final byte zero = (byte) 0b10000000;
        final byte first = 0b01000000;
        final byte seventh = 0b00000001;

        assert Utilities.isBitSet(fifth, 5);
        assert !Utilities.isBitSet(fifth, 4);
        assert !Utilities.isBitSet(fifth, 6);

        assert Utilities.isBitSet(zero, 0);
        assert !Utilities.isBitSet(zero, 1);
        assert !Utilities.isBitSet(zero, 5);
        assert !Utilities.isBitSet(zero, 7);

        assert Utilities.isBitSet(first, 1);
        assert !Utilities.isBitSet(first, 2);
        assert !Utilities.isBitSet(first, 3);
        assert !Utilities.isBitSet(first, 4);
        assert !Utilities.isBitSet(first, 5);
        assert !Utilities.isBitSet(first, 6);
        assert !Utilities.isBitSet(first, 7);
        assert !Utilities.isBitSet(first, 0);

        assert Utilities.isBitSet(seventh, 7);
        assert !Utilities.isBitSet(seventh, 6);
    }

    @Test
    void getBitsFromTo() {
        final byte first = 0b00010100;
        final byte second = (byte) 0b11010010;
        final byte third = 0b01011010;
        final byte fourth = (byte) 0b10000001;

        assert Utilities.getBitsFromTo(first, 3, 6) == 0b00000101;
        assert Utilities.getBitsFromTo(first, 0, 4) == 0b00000001;

        assert Utilities.getBitsFromTo(second, 0, 4) == 0b00001101;
        assert Utilities.getBitsFromTo(second, 5, 8) == 0b00000010;
        assert Utilities.getBitsFromTo(second, 2, 6) == 0b00000100;

        assert Utilities.getBitsFromTo(third, 4, 6) == 0b00000010;

        assert Utilities.getBitsFromTo(first, 0, 8) == first;
        assert Utilities.getBitsFromTo(second, 0, 8) == second;
        assert Utilities.getBitsFromTo(third, 0, 8) == third;
        assert Utilities.getBitsFromTo(fourth, 0, 8) == fourth;
    }

}