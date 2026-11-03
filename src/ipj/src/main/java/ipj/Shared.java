package ipj;

import java.io.InvalidClassException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.AbstractMap;
import java.util.Arrays;
import java.util.Collection;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

import javax.management.RuntimeErrorException;

public abstract class Shared implements AutoCloseable {
	public static class UnshareableClassException
	extends InvalidClassException {
		public UnshareableClassException(Class<?> cls) {
			super(cls.getName());
		}
	}

	private static Map<Class<?>, Layout.Item>
	SUPPORTED_PRIMITIVES = Map.of(
		byte.class, LayoutByteItem.getInstance(),
		short.class, LayoutShortItem.getInstance(),
		int.class, LayoutIntItem.getInstance(),
		long.class, LayoutLongItem.getInstance());

	static { System.loadLibrary("ipjshmem"); }

	private Layout layout;
	private MemoryBlock memory;

	public static <T extends Shared> T
	open(Class<T> cls, String id, Object... initArgs)
		throws NoSuchMethodException, InstantiationException,
		IllegalAccessException, IllegalArgumentException,
		InvocationTargetException, OutOfMemoryError
	{
		validateShareable(cls);
		final var instance = makeInstance(cls, initArgs);
		final Shared shared = instance;

		shared.layout = makeObjectLayout(cls);
		final var isNewObject = Boolean.valueOf(false);
		final var handle = provide(id, shared.layout.getSize(), isNewObject);
		if (handle == 0)
			throw new OutOfMemoryError();

		shared.memory = new MemoryBlock(handle);

		if (isNewObject) {
			final var classInfoLayout = makeClassInfoLayout(cls);

			final var isNewCls = Boolean.valueOf(false);
			final var clsInfoHandle = provide(
				cls.getName(), classInfoLayout.getSize(), isNewCls);
			if (clsInfoHandle == 0) {
				free(handle);
				throw new OutOfMemoryError();
			}

			if (isNewCls) {
				final var clsInfoBlock = new MemoryBlock(clsInfoHandle);
				layout.markupClsInfo(clsInfoBlock);
			}
			layout.markupObj(shared.memory, clsInfoHandle, instance);
		}

		return instance;
	}

	private static void validateShareable(Class<?> cls)
	throws UnshareableClassException {
		final var unshareable = findUnshareable(cls);
		if (unshareable != null)
			throw new UnshareableClassException(unshareable);
	}

	private static Class<?> findUnshareable(Class<?> cls) {
		if (Shared.class.isAssignableFrom(cls)) {
			return Arrays.stream(cls.getDeclaredFields())
				.map(Field::getType)
				.map(Shared::findUnshareable)
				.filter(Objects::nonNull)
				.findFirst()
				.orElse(null);
		} else {
			return isSupportedPrimitive(cls) ? null : cls;
		}
	}

	private static boolean isSupportedPrimitive(Class<?> cls) {
		return SUPPORTED_PRIMITIVES.containsKey(cls);
	}

	private static <T> T makeInstance(Class<T> cls, Object... initArgs)
	throws NoSuchMethodException, InstantiationException,
	IllegalAccessException, IllegalArgumentException,
	InvocationTargetException {
		final var ctrParamTypes = Arrays.stream(initArgs)
			.map((obj) -> obj == null ? Shared.class : obj.getClass())
			.toArray(Class[]::new);
		final var ctr = cls.getDeclaredConstructor(ctrParamTypes);
		return ctr.newInstance(initArgs);
	}

	private static Layout makeObjectLayout(Class<?> cls) {
		try {
			final var layout = new Layout(
				makeLayoutItem("#clsref", LayoutLongItem.getInstance()),
				makeLayoutItem("#refcount", LayoutIntItem.getInstance()));

			final var fieldItems = Arrays
				.stream(cls.getDeclaredFields())
				.map(Shared::makeLayoutItem);
			layout.append(fieldItems::iterator);

			return layout;
		} catch (Layout.IdDuplicationException e) {
			// practically impossible
			throw new RuntimeException(e);
		}
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutItem(Field field) {
		final var fieldType = field.getType();
		final var layoutItem = SUPPORTED_PRIMITIVES.get(fieldType);
		assert layoutItem != null : " unshareable field";
		return makeLayoutItem(field.getName(), layoutItem);
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutItem(String id, Layout.Item item) {
		return new AbstractMap.SimpleImmutableEntry<>(id, item);
	}

	private static Layout makeClassInfoLayout(Class<?> cls) {
		try {
			final var layout = new Layout(
				makeLayoutItem("#clsid", LayoutLongItem.getInstance()),
				makeLayoutItem("#refcount", LayoutIntItem.getInstance()),
				makeLayoutItem("#fieldCount", LayoutIntItem.getInstance()));
			
			for (final var field : cls.getDeclaredFields())
				layout.append(field.getName(), LayoutShortItem.getInstance());

			return layout;
		} catch (Layout.IdDuplicationException e) {
			// practically impossible
			throw new RuntimeException(e);
		}
	}

	@Override
	public void close() {}

	private static native long provide(String id, int size, Boolean isNew);
	private static native long alloc(int size);
	private static native void free(long handle);
}
