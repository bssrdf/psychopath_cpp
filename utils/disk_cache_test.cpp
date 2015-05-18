#include "test.hpp"

#include "disk_cache.hpp"
#include "rng.hpp"


TEST_CASE("disk_cache")
{
	// Constructors
	SECTION("constructor") {
		DiskCache::Cache<float, 1024> cache1(100000, 32);
		DiskCache::Cache<float, 213> cache2(30001, 33);

		REQUIRE(cache1.block_size() == 1024);
		REQUIRE(cache2.block_size() == 213);
		REQUIRE(cache1.element_count() >= 100000);
		REQUIRE(cache2.element_count() >= 30001);
	}

	SECTION("manual_init") {
		DiskCache::Cache<float, 1024> cache1;
		DiskCache::Cache<float, 213> cache2;

		cache1.init(100000, 32);
		cache2.init(30001, 33);

		REQUIRE(cache1.block_size() == 1024);
		REQUIRE(cache2.block_size() == 213);
		REQUIRE(cache1.element_count() >= 100000);
		REQUIRE(cache2.element_count() >= 30001);
	}

	SECTION("write_read") {
		RNG rng(1);
		DiskCache::Cache<float, 1024> cache(1000000, 32);

		for (int i = 0; i < 1000000; i++) {
			cache[i] = rng.next_float();
		}

		rng.seed(1);
		bool match = true;
		for (int i = 0; i < 1000000; i++) {
			match = match && cache[i] == rng.next_float();
		}

		REQUIRE(match);
	}
}

