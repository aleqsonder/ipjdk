import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import ipj.MemoryAllocator;
import ipj.MemoryBlock;
import ipj.SharedMemoryAllocator;

public class MemoryBlockTest {
	private static final MemoryAllocator
	ALLOCATOR = SharedMemoryAllocator.getInstance();

	private MemoryBlock block;

	@BeforeEach
	void setup() {
		block = new MemoryBlock(ALLOCATOR.alloc(32));
	}

	@AfterEach 
	void teardown() {
		ALLOCATOR.release(block.getHandle());
	}

	@Test 
	void io8() {
		final byte value = 107;
		block.set8(value);
		assertEquals(value, block.get8());
	}

	@Test
	void io16() {
		final short value = 4134;
		block.set16(value);
		assertEquals(value, block.get16());
	}

	@Test
	void io32() {
		final int value = 3512534;
		block.set32(value);
		assertEquals(value, block.get32());
	}

	@Test
	void io64() {
		final long value = 41352464526L;
		block.set64(value);
		assertEquals(value, block.get64());
	}

	@Test
	void markup() {
		final byte val8 = 111;
		final short val16 = 4315;
		final int val32 = 8265625;
		final long val64 = 513426234L;

		block.put8(val8).put16(val16).put32(val32).put64(val64);

		assertEquals(val8, block.get8());
		assertEquals(val16, block.skip(1).get16());
		assertEquals(val32, block.skip(3).get32());
		assertEquals(val64, block.skip(7).get64());
	}
}
