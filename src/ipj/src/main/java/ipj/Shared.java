package ipj;

import java.io.InvalidClassException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.security.KeyException;
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
		short.class, LayoutShortItemLP64.getInstance(),
		int.class, LayoutIntItemLP64.getInstance(),
		long.class, LayoutLongItemLP64.getInstance());

	private static String
		CLASS_REF_LABEL = "#clsref",
		REF_COUNT_LABEL = "#refcnt",
		CLASS_ID_LABEL = "#clsid",
		FIELD_COUNT_LABEL = "#fldcnt";

	static { System.loadLibrary("ipjshmem"); }

	private Layout layout;
	private MemoryBlockLP64 memory;

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

		shared.memory = new MemoryBlockLP64(handle);

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
				final var clsInfoBlock = new MemoryBlockLP64(clsInfoHandle);
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
				// FIXME: Use pointer instead of ulong
				makeLayoutLongEntry(CLASS_REF_LABEL),
				makeLayoutIntEntry(REF_COUNT_LABEL));

			final var fieldItems = Arrays
				.stream(cls.getDeclaredFields())
				.map(Shared::makeLayoutEntry);
			layout.append(fieldItems::iterator);

			return layout;
		} catch (Layout.IdDuplicationException e) {
			// practically impossible
			throw new RuntimeException(e);
		}
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutEntry(Field field) {
		final var fieldType = field.getType();
		final var layoutItem = SUPPORTED_PRIMITIVES.get(fieldType);
		assert layoutItem != null : " unshareable field";
		return makeLayoutEntry(field.getName(), layoutItem);
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutEntry(String id, Layout.Item item) {
		return new AbstractMap.SimpleImmutableEntry<>(id, item);
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutShortEntry(String id) {
		return makeLayoutEntry(id, LayoutShortItemLP64.getInstance());
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutIntEntry(String id) {
		return makeLayoutEntry(id, LayoutIntItemLP64.getInstance());
	}

	private static Map.Entry<String, Layout.Item>
	makeLayoutLongEntry(String id) {
		return makeLayoutEntry(id, LayoutLongItemLP64.getInstance());
	}

	private static Layout makeClassInfoLayout(Class<?> cls) {
		try {
			final var layout = new Layout(
				makeLayoutLongEntry(CLASS_ID_LABEL),
				makeLayoutIntEntry(REF_COUNT_LABEL),
				makeLayoutIntEntry(FIELD_COUNT_LABEL));
			
			for (final var field : cls.getDeclaredFields())
				layout.append(field.getName(), LayoutShortItemLP64.getInstance());

			return layout;
		} catch (Layout.IdDuplicationException e) {
			// practically impossible
			throw new RuntimeException(e);
		}
	}

	@Override
	public void close() {
		try {
			final var refCountBlock = memory.skip(layout.getOffset(REF_COUNT_LABEL));
			final var nextRefcount = refCountBlock.getInt() - 1;
			refCountBlock.set(nextRefcount);

			if (nextRefcount == 0) {
				final var classRefBlock = layout.skip(memory, CLASS_REF_LABEL);
				// final var classInfoBlock = classRefBlock.get
			}
		} catch (KeyException e) {
			throw new RuntimeException(e);
		}
	}

	private void addRef() {
		final var refCountBlock = getRefCountBlock();
		refCountBlock.set(refCountBlock.getInt() + 1);
	}

	private void release() {
		final var refCountBlock = getRefCountBlock();
		final var nextRefCount = refCountBlock.getInt() - 1;
		refCountBlock.set(nextRefCount);

		if (nextRefCount == 0)
			// TODO ?
	}

	private MemoryBlockLP64 getRefCountBlock() {
		try {
			return memory.skip(layout.getOffset(REF_COUNT_LABEL));
		} catch (KeyException e) {
			throw new RuntimeException(e);
		}
	}

	private static native long provide(String id, int size, Boolean isNew);
	private static native long alloc(int size);
	private static native void free(long handle);
}
