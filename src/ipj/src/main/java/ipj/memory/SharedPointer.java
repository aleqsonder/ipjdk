package ipj.memory;

/**
 * Pointer into a shared memory region.
 *
 * A pointer made by a public constructor owns a reference to
 * the region and releases it on {@link #close()}.
 */
public final class SharedPointer implements Pointer, AutoCloseable {
	static {
		System.loadLibrary("ipjshmem");
	}

	private static final long INVALID_HANDLE = 0;

	private long handle;
	private final long offsetBytes;
	private final boolean isNew;

	/**
	 * Creates or opens the named shared region.
	 * Use {@link #isNew()} to find out whether it already existed.
	 */
	public SharedPointer(String id, long size) {
		final long newHandle = provide(id, size);
		validateHandle(newHandle);

		this.handle = Math.abs(newHandle);
		this.offsetBytes = 0;
		this.isNew = newHandle < 0;
	}

	/** Creates a new anonymous shared region. */
	public SharedPointer(long size) {
		final long newHandle = alloc(size);
		validateHandle(newHandle);

		this.handle = newHandle;
		this.offsetBytes = 0;
		this.isNew = true;
	}

	private SharedPointer(long handle, long offset) {
		this.handle = handle;
		this.offsetBytes = offset;
		this.isNew = false;
	}

	private static void validateHandle(long handle) {
		if (handle == INVALID_HANDLE)
			throw new OutOfMemoryError();
	}

	/**
	 * Returns {@code true} if the region was created by the
	 * constructor of this pointer, {@code false} if it already existed.
	 */
	public boolean isNew() { return isNew; }

	public long getHandle() { return handle; }
	public long getOffsetBytes() { return offsetBytes; }

	@Override
	public SharedPointer skip(long offsetBytesDelta) {
		final var newOffsetBytes = offsetBytes + offsetBytesDelta;
		return new SharedPointer(handle, newOffsetBytes);
	}

	@Override
	public void close() {
		if (handle != INVALID_HANDLE) {
			release(handle);
			handle = INVALID_HANDLE;
		}
	}


	// byte

	@Override
	public byte get8() { return get8(handle, offsetBytes); }

	@Override
	public void set8(byte value) { set8(handle, offsetBytes, value); }

	@Override
	public SharedPointer put8(byte value) {
		set8(value);
		return skip(Byte.BYTES);
	}


	// short

	@Override
	public short get16() { return get16(handle, offsetBytes); }

	@Override
	public void set16(short value) { set16(handle, offsetBytes, value); }

	@Override
	public SharedPointer put16(short value) {
		set16(value);
		return skip(Short.BYTES);
	}


	// int

	@Override
	public int get32() { return get32(handle, offsetBytes); }

	@Override
	public void set32(int value) { set32(handle, offsetBytes, value); }

	@Override
	public SharedPointer put32(int value) {
		set32(value);
		return skip(Integer.BYTES);
	}


	// long

	@Override
	public long get64() { return get64(handle, offsetBytes); }

	@Override
	public void set64(long value) { set64(handle, offsetBytes, value); }

	@Override
	public SharedPointer put64(long value) {
		set64(value);
		return skip(Long.BYTES);
	}


	// native

	/**
	 * Creates or opens the named region. A negative result means the
	 * region was created by this call; negate it to get the handle.
	 */
	private static native long provide(String id, long size);

	private static native long alloc(long size);
	private static native void release(long handle);

	private static native byte get8(long handle, long offset);
	private static native short get16(long handle, long offset);
	private static native int get32(long handle, long offset);
	private static native long get64(long handle, long offset);

	private static native void set8(long handle, long offset, byte value);
	private static native void set16(long handle, long offset, short value);
	private static native void set32(long handle, long offset, int value);
	private static native void set64(long handle, long offset, long value);
}
