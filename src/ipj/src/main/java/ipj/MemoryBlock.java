package ipj;

public interface MemoryBlock {
	MemoryBlock skip(long offsetDelta);

	// byte
	default MemoryBlock put(byte value) { set(value); return skip(1); }
	void set(byte value);
	byte getByte();

	// short
	MemoryBlock put(short value);
	void set(short value);
	short getShort();

	// int
	MemoryBlock put(int value);
	void set(int value);
	int getInt();

	// long
	MemoryBlock put(long value);
	void set(long value);
	long getLong();
}
