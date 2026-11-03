package ipj;

public class MemoryBlockLP64 implements MemoryBlock {
	private long handle, offset;

	public MemoryBlockLP64(long handle) { this(handle, 0); }

	public MemoryBlockLP64(long handle, long offset) {
		this.handle = handle;
		this.offset = offset;
	}

	public long getHandle() { return handle; };

	public MemoryBlockLP64 skip(long offsetDelta) {
		return new MemoryBlockLP64(handle, offset + offsetDelta);
	}


	// byte

	@Override
	public void set(byte value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, byte value);

	@Override
	public byte getByte() { return getByte(handle, offset); }
	private static native byte getByte(long handle, long offset);


	// short
	
	@Override
	public MemoryBlockLP64 put(short value) {
		set(value);
		return skip(LP64.SHORT_SIZE);
	}

	@Override
	public void set(short value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, short value);

	@Override
	public short getShort() { return getShort(handle, offset); }
	private static native short getShort(long handle, long offset);


	// int

	@Override 
	public MemoryBlockLP64 put(int value) {
		set(value);
		return skip(LP64.INT_SIZE);
	}

	@Override 
	public void set(int value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, int value);

	@Override 
	public int getInt() { return getInt(handle, offset); }
	private static native int getInt(long handle, long offset);


	// long

	@Override 
	public MemoryBlockLP64 put(long value) {
		set(value);
		return skip(LP64.LONG_SIZE);
	}

	@Override 
	public void set(long value) { set(handle, offset, value); }
	private static native void set(long handle, long offset, long value);

	@Override 
	public long getLong() { return getLong(handle, offset); }
	private static native long getLong(long handle, long offset);


	// pointer

	public final MemoryBlockLP64 put(MemoryBlockLP64 pointer) {
		set(pointer);
		return skip(LP64.LONG_SIZE);
	}

	public final void set(MemoryBlockLP64 pointer) {
		set(queryHandle(handle, offset));
	}
	private static native long queryHandle(long handle, long offset);

	public final MemoryBlockLP64 getPointer() {
		final var handle = getLong();
		return new MemoryBlockLP64(handle);
	}
};