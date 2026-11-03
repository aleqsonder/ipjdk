package ipj;

import java.io.InvalidClassException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Map;
import java.util.stream.Collectors;

public abstract class Shared implements AutoCloseable {
	public static class UnshareableClassException
	extends InvalidClassException {
		public UnshareableClassException(Class<?> cls) {
			super(cls.getName());
		}
	}

	public static class UnknownPrimitiveException
	extends InvalidClassException {
		public UnknownPrimitiveException(Class<?> cls) {
			super(cls.getName());
		}
	}

	private static class Layout {
		public static final int
			BYTE_SIZE = 1,
			SHORT_SIZE = 2,
			INT_SIZE = 4,
			LONG_SIZE = 8,
			FLOAT_SIZE = 4,
			DOUBLE_SIZE = 8,
			PTR_SIZE = 8;

		// all except for char
		private static final Map<Class<?>, Integer>
		PRIMITIVE_SIZES = Map.of(
			byte.class, BYTE_SIZE,
			short.class, SHORT_SIZE,
			int.class, INT_SIZE,
			long.class, LONG_SIZE,
			float.class, FLOAT_SIZE,
			double.class, DOUBLE_SIZE);

		private Class<? extends Shared> cls;

		public Layout(Class<? extends Shared> cls)
		throws InvalidClassException {
			validateShareable(cls);
			this.cls = cls;
		}

		private static void validateShareable(Class<?> cls)
		throws InvalidClassException {
			if (cls.isPrimitive()) {
				if (!PRIMITIVE_SIZES.containsKey(cls))
					throw new UnknownPrimitiveException(cls);
			} else if (cls.isArray()) {
				validateShareable(cls.arrayType());
			} else if (Shared.class.isAssignableFrom(cls)) {
				for (final var field : cls.getDeclaredFields())
					validateShareable(field.getType());
			} else {
				throw new UnshareableClassException(cls);
			}
		}

		private static int sizeof(Class<?> cls) {
			if (cls.isPrimitive()) {
				final var size = PRIMITIVE_SIZES.get(cls);
				assert size != null : " unknown primitive";
				return size;
			}

			if (cls.isArray() || Shared.class.isAssignableFrom(cls))
				return PTR_SIZE;

			return getFieldsSize(cls);
		}

		private static int getFieldsSize(Class<?> cls) {
			return getFieldTypes(cls).stream()
				.map(Layout::sizeof)
				.reduce(0, Integer::sum);
		}

		private static Collection<Class<?>> getFieldTypes(Class<?> cls) {
			return getFields(cls).stream()
				.map(Field::getType)
				.collect(Collectors.toUnmodifiableList());
		}

		private static Collection<Field> getFields(Class<?> cls) {
			return Arrays.asList(cls.getDeclaredFields());
		}

		public int getObjSize() {
			final var clsrefSize = LONG_SIZE;
			final var refcountSize = INT_SIZE;
			return clsrefSize + refcountSize + getFieldsSize(cls);
		}

		public int getClsInfoSize() {
			final var clsidSize = LONG_SIZE;
			return clsidSize + getFieldOffsetTableSize();
		}

		private int getFieldOffsetTableSize() {
			return getFields(cls).size() * SHORT_SIZE;
		}

		public void markupObj(
			MemoryBlock objBlock,
			long clsInfoHandle,
			Object instance
		) throws IllegalArgumentException, IllegalAccessException {
			objBlock = objBlock.put(clsInfoHandle);
			for (final var field : getFields(cls))
				objBlock = putField(objBlock, field, instance);
		}

		public void markupClsInfo(MemoryBlock clsInfoBlock) {
			final var fields = getFields(cls);
			clsInfoBlock = clsInfoBlock
				.put(getClsid(cls))
				.put(fields.size());

			short offset = 0;
			for (final var field : fields) {
				clsInfoBlock = clsInfoBlock.put(offset);
				offset += sizeof(field.getType());
			}
		}

		private static long getClsid(Class<? extends Shared> cls) {
			try {
				final var field = cls.getDeclaredField("IPJ_CLSID");
				return field.getLong(null);
			} catch (NoSuchFieldException
				| IllegalArgumentException
				| IllegalAccessException e
			) { return calcClsid(cls); }
		}

		private static long calcClsid(Class<? extends Shared> cls) {
			throw new UnsupportedOperationException("not supported yet");
		}

		private static MemoryBlock
		putField(MemoryBlock block, Field field, Object obj)
		throws IllegalArgumentException, IllegalAccessException {
			final var fieldType = field.getType();
			if (!fieldType.isPrimitive()) {
				throw new UnsupportedOperationException(
					"non-primitive fields are not supported yet");
			}

			if (fieldType == byte.class)
				return block.put(field.getByte(obj));
			if (fieldType == short.class)
				return block.put(field.getShort(obj));
			if (fieldType == int.class)
				return block.put(field.getInt(obj));
			if (fieldType == long.class)
				return block.put(field.getLong(obj));

			throw new UnsupportedOperationException(
				obj.getClass().getName() + " is not supported yet");
		}
	};

	private static class MemoryBlock {
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
			return skip(Layout.BYTE_SIZE);
		}
		public void set(byte value) { set(handle, offset, value); }
		private static native void set(long handle, long offset, byte value);

		// short
		public MemoryBlock put(short value) {
			set(value);
			return skip(Layout.SHORT_SIZE);
		}
		public void set(short value) { set(handle, offset, value); }
		private static native void set(long handle, long offset, short value);

		// int
		public MemoryBlock put(int value) {
			set(value);
			return skip(Layout.INT_SIZE);
		}
		public void set(int value) { set(handle, offset, value); }
		private static native void set(long handle, long offset, int value);

		// long
		public MemoryBlock put(long value) {
			set(value);
			return skip(Layout.LONG_SIZE);
		}
		public void set(long value) { set(handle, offset, value); }
		private static native void set(long handle, long offset, long value);
	};

	static { System.loadLibrary("ipjshmem"); }

	private MemoryBlock obj;

	public static <T extends Shared> T
	open(Class<T> cls, String id, Object... initArgs)
		throws NoSuchMethodException, InstantiationException,
		IllegalAccessException, IllegalArgumentException,
		InvocationTargetException, InvalidClassException,
		OutOfMemoryError
	{
		final var ctrParamTypes = Arrays.stream(initArgs)
			.map((obj) -> obj == null ? Shared.class : obj.getClass())
			.toArray(Class[]::new);
		final var ctr = cls.getDeclaredConstructor(ctrParamTypes);
		final var instance = ctr.newInstance(initArgs);

		final var layout = new Layout(cls);
		final var isNew = Boolean.valueOf(false);
		final var handle = provide(id, layout.getObjSize(), isNew);
		if (handle == 0)
			throw new OutOfMemoryError();

		final Shared shared = instance;
		shared.obj = new MemoryBlock(handle);

		if (isNew) {
			final var isNewCls = Boolean.valueOf(false);
			final var clsInfoHandle = provide(
				cls.getName(), layout.getClsInfoSize(), isNewCls);
			if (clsInfoHandle == 0) {
				free(handle);
				throw new OutOfMemoryError();
			}
			if (isNewCls) {
				final var clsInfoBlock = new MemoryBlock(clsInfoHandle);
				layout.markupClsInfo(clsInfoBlock);
			}
			layout.markupObj(shared.obj, clsInfoHandle, instance);
		}

		return instance;
	}

	@Override
	public void close() {}

	private static native long provide(String id, int size, Boolean isNew);
	private static native long alloc(int size);
	private static native void free(long handle);
}
