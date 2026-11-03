package ipj;

public class LayoutIntItemLP64 implements Layout.Item {
	private static final LayoutIntItemLP64
	INSTANCE = new LayoutIntItemLP64();

	public static LayoutIntItemLP64 getInstance() { return INSTANCE; }

	private LayoutIntItemLP64() {}

	@Override
	public int getSize() { return LP64.INT_SIZE; }
}
