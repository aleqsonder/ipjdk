package ipj;

public class LayoutIntItem implements Layout.Item {
	private static final LayoutIntItem INSTANCE = new LayoutIntItem();
	public static LayoutIntItem getInstance() { return INSTANCE; }

	private LayoutIntItem() {}

	@Override
	public int getSize() { return MemoryBlock.INT_SIZE; }
}
