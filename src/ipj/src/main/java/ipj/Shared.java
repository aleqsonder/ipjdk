package ipj;

import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;

public abstract class Shared implements AutoCloseable {
	private long ptr = 0;
	private String id = null;

	public static <T extends Shared> T
	open(Class<T> cls, String id, Object... initArgs)
	throws NoSuchMethodException, InstantiationException,
	IllegalAccessException, IllegalArgumentException,
	InvocationTargetException {
		final var ctrParamTypes = Arrays.stream(initArgs)
			.map((obj) -> obj == null ? Shared.class : obj.getClass())
			.toArray(Class[]::new);
		final var ctr = cls.getDeclaredConstructor(ctrParamTypes);
		final var instance = ctr.newInstance(initArgs);
		((Shared)instance).id = id;
		return instance;
	}

	public static <T extends Shared> void release(T obj) {}

	@Override
	public void close() { release(this); }
}
