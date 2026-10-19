package ipj;

import java.io.InvalidClassException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.logging.Logger;

public abstract class Shared implements AutoCloseable {
	private long ptr = 0;
	private String id = null;

	private static final Logger
	LOGGER = Logger.getLogger(Shared.class.getName());

	private static final int
		PTR_SIZE = 8,
		INT_SIZE = 4,
		LONG_SIZE = 8;

	public static <T extends Shared> T
	open(Class<T> cls, String id, Object... initArgs)
	throws NoSuchMethodException, InstantiationException,
	IllegalAccessException, IllegalArgumentException,
	InvocationTargetException, InvalidClassException,
	OutOfMemoryError {
		final var size = sizeof(cls);

		final var ctrParamTypes = Arrays.stream(initArgs)
			.map((obj) -> obj == null ? Shared.class : obj.getClass())
			.toArray(Class[]::new);
		final var ctr = cls.getDeclaredConstructor(ctrParamTypes);
		final var instance = ctr.newInstance(initArgs);

		final Shared shared = instance;
		shared.id = id;
		shared.ptr = alloc(size);

		if (shared.ptr == 0)
			throw new OutOfMemoryError();

		return instance;
	}

	@Override
	public void close() {}

	@Override
	public String toString() {
		return "Shared(ptr=" + ptr + ", id=\""+ id + "\")";
	}

	private static <T extends Shared> int sizeof(Class<T> cls)
	throws InvalidClassException {
		int size = 0;
		for (final var field : cls.getDeclaredFields())
			size += sizeof(field);
		return size;
	}

	private static int sizeof(Field field) throws InvalidClassException {
		final var cls = field.getType();
		if (!cls.isPrimitive())
			return PTR_SIZE;

		if (cls == int.class) return INT_SIZE;
		if (cls == long.class) return LONG_SIZE;

		final var msg = field.getName() + ": " + cls.getName();
		throw new InvalidClassException(msg);
	}

	private static native long alloc(int size);
	private static native void free(long ptr);
}
