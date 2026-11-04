package ipj;

public interface MemoryAllocator {
	public record Provision(long handle, boolean isNew) {}

	long alloc(long size);
	Provision provide(String id, long size);
	void release(long handle);
}
