package ipj;

public class MemoryBlock {
	public static int
		BYTE_SIZE = 1,
		SHORT_SIZE = 2,
		INT_SIZE = 4,
		LONG_SIZE = 8;

	private long handle, offset;

	public MemoryBlock(long handle) { this(handle, 0); }

	public MemoryBlock(long handle, long offset) {
		this.handle = handle;
		this.offset = offset;
	}

	public long getHandle() { return handle; }

	public MemoryBlock skip(long offsetDelta) {
		return new MemoryBlock(handle, offset + offsetDelta);
	}

	// byte
	public MemoryBlock put(byte value) {
		set(value);
		return skip(BYTE_SIZE);
	}
	public void set(byte value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, byte value);

	// short
	public MemoryBlock put(short value) {
		set(value);
		return skip(SHORT_SIZE);
	}
	public void set(short value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, short value);

	// int
	public MemoryBlock put(int value) {
		set(value);
		return skip(INT_SIZE);
	}
	public void set(int value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, int value);

	// long
	public MemoryBlock put(long value) {
		set(value);
		return skip(LONG_SIZE);
	}
	public void set(long value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, long value);
};