package ipj;

public class MemoryBlock {
	static { System.loadLibrary("ipjshmem"); }

	private long handle, offsetBytes;

	public MemoryBlock(long handle) { this(handle, 0); }

	public MemoryBlock(long handle, long offsetBytes) {
		this.handle = handle;
		this.offsetBytes = offsetBytes;
	}

	public long getHandle() { return handle; };

	public MemoryBlock skip(long offsetBytesDelta) {
		return new MemoryBlock(handle, offsetBytes + offsetBytesDelta);
	}


	// byte

	public final MemoryBlock put8(byte value) {
		set8(value);
		return skip(1);
	}

	public final void set8(byte value) {
		set8(handle, offsetBytes, value);
	}

	private static native void
	set8(long handle, long offsetBytes, byte value);

	public final byte get8() { return get8(handle, offsetBytes); }
	private static native byte get8(long handle, long offsetBytes);


	// short

	public final MemoryBlock put16(short value) {
		set16(value);
		return skip(2);
	}

	public final void set16(short value) {
		set16(handle, offsetBytes, value);
	}

	private static native void
	set16(long handle, long offsetBytes, short value);

	public final short get16() { return get16(handle, offsetBytes); }
	private static native short get16(long handle, long offsetBytes);


	// int

	public final MemoryBlock put32(int value) {
		set32(value);
		return skip(4);
	}

	public final void set32(int value) {
		set32(handle, offsetBytes, value);
	}

	private static native void
	set32(long handle, long offsetBytes, int value);

	public final int get32() { return get32(handle, offsetBytes); }
	private static native int get32(long handle, long offsetBytes);


	// long

	public final MemoryBlock put64(long value) {
		set64(value);
		return skip(8);
	}

	public final void set64(long value) {
		set64(handle, offsetBytes, value);
	}

	private static native void
	set64(long handle, long offsetBytes, long value);

	public final long get64() { return get64(handle, offsetBytes); }
	private static native long get64(long handle, long offsetBytes);
};