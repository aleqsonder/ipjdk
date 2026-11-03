package ipj;

public class LayoutShortItem implements Layout.Item {
	private static final LayoutShortItem
	INSTANCE = new LayoutShortItem();

	public static LayoutShortItem getInstance() { return INSTANCE; }

	private LayoutShortItem() {}

	@Override
	public int getSize() { return MemoryBlock.SHORT_SIZE; }
}
