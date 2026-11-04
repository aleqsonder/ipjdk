package ipj;

public class SharedMemoryAllocator implements MemoryAllocator {
	private static final SharedMemoryAllocator
	INSTANCE = new SharedMemoryAllocator();

	public static SharedMemoryAllocator getInstance() {
		return INSTANCE;
	}

	private static long INVALID_HANDLE = 0xffffffffffffffffL;

	private static native long shalloc(long size);

	// a negative result stands for a newly created object
	private static native long shprovide(String id, long size);
	
	private static native void shrelease(long handle);

	static { System.loadLibrary("ipjshmem"); }

	private SharedMemoryAllocator() {}

	@Override
	public long alloc(long size) {
		final var handle = shalloc(size);
		if (handle == INVALID_HANDLE)
			throw new OutOfMemoryError();
		return handle;
	}

	@Override
	public Provision provide(String id, long size) {
		final var handle = shprovide(id, size);
		if (handle == INVALID_HANDLE)
			throw new OutOfMemoryError();
		return handle < 0
			? new Provision(-handle, true)
			: new Provision(handle, false);
	}

	@Override
	public void release(long handle) { shrelease(handle); }
}
