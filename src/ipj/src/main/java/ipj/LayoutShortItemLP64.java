package ipj;

public class LayoutShortItemLP64 implements Layout.Item {
	private static final LayoutShortItemLP64
	INSTANCE = new LayoutShortItemLP64();

	public static LayoutShortItemLP64 getInstance() { return INSTANCE; }

	private LayoutShortItemLP64() {}

	@Override
	public int getSize() { return LP64.SHORT_SIZE; }
}
