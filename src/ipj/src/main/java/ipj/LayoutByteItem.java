package ipj;

public class LayoutByteItem implements Layout.Item {
	private static final LayoutByteItem INSTANCE = new LayoutByteItem();
	public static LayoutByteItem getInstance() { return INSTANCE; }

	private LayoutByteItem() {}

	@Override
	public int getSize() { return 1; }
}
