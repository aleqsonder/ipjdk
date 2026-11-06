package ipj.memory;

/**
 * Typed access to memory at some position, like a C pointer.
 *
 * <p>{@link #skip(long)} works like pointer arithmetic: it returns
 * a new pointer moved by the given number of bytes and leaves this
 * one as is.
 *
 * <p>The {@code put*} methods write a value and return a pointer
 * right past it, so writes can be chained. Implementations that want
 * chaining to keep their own type should override {@code skip} and
 * every {@code put*} with a covariant return type.
 */
public interface Pointer {
	/**
	 * Returns a new pointer moved by {@code offsetBytes}.
	 * This pointer is not changed.
	 */
	Pointer skip(long offsetBytes);


	// byte

	byte get8();
	void set8(byte value);

	default Pointer put8(byte value) {
		set8(value);
		return skip(Byte.BYTES);
	}


	// short

	short get16();
	void set16(short value);

	default Pointer put16(short value) {
		set16(value);
		return skip(Short.BYTES);
	}


	// int

	int get32();
	void set32(int value);

	default Pointer put32(int value) {
		set32(value);
		return skip(Integer.BYTES);
	}


	// long

	long get64();
	void set64(long value);

	default Pointer put64(long value) {
		set64(value);
		return skip(Long.BYTES);
	}
}
