package ipj;

public class LayoutLongItem implements Layout.Item {
	private static final LayoutLongItem INSTANCE = new LayoutLongItem();
	public static LayoutLongItem getInstance() { return INSTANCE; }

	private LayoutLongItem() {}

	@Override
	public int getSize() { return MemoryBlock.LONG_SIZE; }
}
