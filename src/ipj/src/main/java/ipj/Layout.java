package ipj;

import java.util.List;
import java.security.KeyException;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

public class Layout {
	public static class IdDuplicationException extends KeyException {
		public IdDuplicationException(String id) { super(id); }
	}

	public static interface Item { int getSize(); }

	private static record Entry(int offset, Item item) {
		public Entry(int offset, Item item) {
			this.offset = offset;
			this.item = item;
		}

		public final int getNextOffset() {
			return offset + item.getSize();
		}
	};

	private List<Entry> entries = new ArrayList<>();
	private Map<String, Integer> itemIndices = new HashMap<>();

	public Layout() {}

	@SafeVarargs
	public Layout(Map.Entry<String, Item>... entries)
	throws IdDuplicationException {
		for (final var entry : entries)
			append(entry.getKey(), entry.getValue());
	}

	public final Layout append(Iterable<Map.Entry<String, Item>> entries)
	throws IdDuplicationException {
		for (final var entry : entries)
			append(entry.getKey(), entry.getValue());
		return this;
	}

	public final Layout append(String id, Item item)
	throws IdDuplicationException {
		final var nextIndex = entries.size();
		if (itemIndices.putIfAbsent(id, nextIndex) != null)
			throw new IdDuplicationException(id);

		entries.add(new Entry(getSize(), item));
		return this;
	}

	public final int getSize() {
		return entries.isEmpty() ? 0 : entries.getLast().getNextOffset();
	};

	public final int getOffset(String id) throws KeyException {
		final var index = itemIndices.get(id);
		if (index == null)
			throw new KeyException(id);
		return entries.get(index).offset();
	}
}
