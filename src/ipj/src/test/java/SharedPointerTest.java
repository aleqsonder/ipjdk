import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import ipj.memory.SharedPointer;

public class SharedPointerTest {
	private SharedPointer ptr;

	@BeforeEach
	void setup() {
		ptr = new SharedPointer(32);
	}

	@AfterEach
	void teardown() {
		ptr.close();
	}

	@Test
	void io8() {
		final byte value = 107;
		ptr.set8(value);
		assertEquals(value, ptr.get8());
	}

	@Test
	void io16() {
		final short value = 4134;
		ptr.set16(value);
		assertEquals(value, ptr.get16());
	}

	@Test
	void io32() {
		final int value = 3512534;
		ptr.set32(value);
		assertEquals(value, ptr.get32());
	}

	@Test
	void io64() {
		final long value = 41352464526L;
		ptr.set64(value);
		assertEquals(value, ptr.get64());
	}

	@Test
	void markup() {
		final byte val8 = 111;
		final short val16 = 4315;
		final int val32 = 8265625;
		final long val64 = 513426234L;

		ptr.put8(val8).put16(val16).put32(val32).put64(val64);

		assertEquals(val8, ptr.get8());
		assertEquals(val16, ptr.skip(1).get16());
		assertEquals(val32, ptr.skip(3).get32());
		assertEquals(val64, ptr.skip(7).get64());
	}

	@Test
	void anonymousIsNew() {
		assertTrue(ptr.isNew());
	}

	@Test
	void namedIsNewOnlyOnce() {
		final String id = "ipj-test-" + ProcessHandle.current().pid();
		try (
			var first = new SharedPointer(id, 16);
			var second = new SharedPointer(id, 16)
		) {
			assertTrue(first.isNew());
			assertFalse(second.isNew());

			first.set32(42);
			assertEquals(42, second.get32());
		}
	}

	@Test
	void derivedCloseKeepsRegion() {
		final int value = 777;
		final SharedPointer derived = ptr.skip(4);
		assertFalse(derived.isNew());

		derived.set32(value);
		derived.close();
		assertEquals(value, ptr.skip(4).get32());
	}
}
