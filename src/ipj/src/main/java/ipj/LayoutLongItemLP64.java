package ipj;

public class LayoutLongItemLP64 implements Layout.Item {
	private static final LayoutLongItemLP64
	INSTANCE = new LayoutLongItemLP64();

	public static LayoutLongItemLP64 getInstance() { return INSTANCE; }

	private LayoutLongItemLP64() {}

	@Override
	public int getSize() { return LP64.LONG_SIZE; }
}
